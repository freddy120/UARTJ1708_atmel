/*
 * uart1.cpp
 *
 * Created: 01/08/2017 10:07:45 a.m.
 *  Author: Freddy
 */ 


#define F_CPU 16000000UL  // 16 MHz
#define BAUD 9600
#include <util/setbaud.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include <stdio.h>
#include <stdint.h>

#include "uart1.h"

#define j1708_my_mid 128 // change this value to change the MID of the module , all modules on the bus should have a different MID



static uint8_t* j1708_tx_buffer; // max [20] bytes;
static uint8_t j1708_tx_length; // bytes to tx
static uint8_t j1708_tx_ptr;

volatile uint8_t* j1708_rx_buffer0;//[21];
volatile uint8_t j1708_rx_buffer0_count;
volatile uint8_t j1708_rx_buffer0_ptr;

volatile uint8_t* j1708_rx_buffer1;//[21];
volatile uint8_t j1708_rx_buffer1_count;
volatile uint8_t j1708_rx_buffer1_ptr;

volatile uint8_t j1708_checksum;
volatile uint8_t j1708_rx_temp;
volatile uint8_t j1708_collision_counter;
volatile uint8_t j1708_rx_limit;

volatile uint8_t bit_times = 1;  // values from 1-10
volatile uint8_t count_times;


struct j1708_status {
	volatile uint8_t j1708_rx_check_mid;		//j1708_status,0
	volatile uint8_t j1708_transmitting;		//j1708_status,1
	volatile uint8_t j1708_tx_busy;   			//j1708_status,2
	volatile uint8_t j1708_active_buffer;		//j1708_status,3
	volatile uint8_t j1708_busy;				//j1708_status,4
	volatile uint8_t j1708_checksum_error;		//j1708_status,5
	volatile uint8_t j1708_rx_overflow;			//j1708_status,6
	volatile uint8_t j1708_tx_sent_checksum;	//j1708_status,7
} bus_status;



//TIMER0

volatile bool flag_count = false;

void config_timer0(void){
	// Prescaler = FCPU/8
	TCCR0A |= (1<<CS01);//(1<<CS02) | (1<<CS00);

	
	//Enable Overflow Interrupt Enable
	//TIMSK0|=(1<<TOIE0);
	
	//enable global interrupt
	sei();
	
	//Initialize Counter
	//TCNT0=0; //127.5 us overflow
	TCNT0=47; //104 us overflow   1 bit time
}


ISR(TIMER0_OVF_vect)
{
	//This is the interrupt service routine for TIMER0 OVERFLOW Interrupt.
	//CPU automatically call this when TIMER0 overflows.
	//Increment our variable
	TCNT0=47; //104 us overflow   1 bit time
	
	count_times++;
	if(count_times==bit_times) //reach n bit times
	{
		
		TIMSK0 &= ~(1<<TOIE0); // disable timer isr
		count_times = 0;
		handle_times_isr();
	}
}

//used to generate an interrupt on the end of a packet AND as a transmit collision retry timer
void handle_times_isr(){

	
	bus_status.j1708_busy = 0; // not busy
	if (bus_status.j1708_transmitting){
		j1708_tx_data();
	}else{  //we are not transmitting so this must be the end of a received packet...time to cleanup
		j1708_rx_limit = 21;
		bus_status.j1708_rx_overflow = 0;
		
		if(j1708_checksum==0){ //valid
			if(bus_status.j1708_active_buffer){
				handle_buffer1();
			}else{
				handle_buffer0();
			}
		}else{
			bus_status.j1708_checksum_error = 1;
		}
		
	}
	
}

void handle_buffer1(){
	bus_status.j1708_active_buffer = 0;
	j1708_rx_buffer0_ptr = 0;
	if (bus_status.j1708_tx_busy){
		j1708_tx_data();
	}
	
}

void handle_buffer0(){
	bus_status.j1708_active_buffer = 1;
	j1708_rx_buffer1_ptr = 0;
	if (bus_status.j1708_tx_busy){
		j1708_tx_data();
	}
}


//Variables for USART1, J1708 bus.
static uint8_t send_pos1;
static uint8_t send_num1;
static uint8_t* send_buf1;


/**
 * \brief UART data register empty interrupt handler
 *
 * This handler is called each time the UART data register is available for
 * sending data.
 */
ISR(USART1_UDRE_vect)
{
	if(j1708_tx_length > j1708_tx_ptr){
		UDR1= j1708_tx_buffer[j1708_tx_ptr];
		j1708_checksum += j1708_tx_buffer[j1708_tx_ptr]; // add to checksum
		j1708_tx_ptr++;
	} else {
		j1708_tx_length = 0;
		//disable sending interrupt
		UCSR1B &= ~(1 << UDRIE1); //Disable sending
		send_checksum();
	}

	/*if(send_num1 > send_pos1){
		UDR1= send_buf1[send_pos1];
		send_pos1++;
	} else {
		send_num1= 0;
		//disable sending interrupt
		UCSR1B &= ~(1 << UDRIE1); //Disable sending
	}*/
	
}

/**
 * \brief Data RX interrupt handler
 *
 * This is the handler for UART receive data
 */
ISR(USART1_RX_vect)
{
	
	uint8_t temp;
	temp = UDR1; // read buffer
	j1708_rx_temp = temp; // save on temp
	
	if (bus_status.j1708_transmitting){ //  yes, so we must..
		if(bus_status.j1708_tx_sent_checksum){
			bus_status.j1708_transmitting = 0;
			bus_status.j1708_tx_sent_checksum = 0;
		}else{
			rx_must_check_mid();
		}
	}else{ // no, so go receive this data
		j1708_rx_isr_receiving();
	}
	
	//UDR1=temp; //tx byte
	//UDR1 = temp;
	//flag_rx = true;
	
}


void send_checksum(){
	uint8_t cs;
	cs = ~(j1708_checksum);
	cs += 1;
	UDR1 = cs; // send checksum
	
	//turn off tx and rx interrupt
	UCSR1B &= ~(1 << UDRIE1);  //disable TX isr 
	UCSR1B &= ~(1 << RXEN1);  //disable RX isr
	
	bus_status.j1708_tx_busy = 0; // tx queue is now empty
	bus_status.j1708_transmitting = 0;
	bus_status.j1708_tx_sent_checksum = 1;
	
}


void j1708_rx_isr_receiving(){
	
	bit_times = 10; //reset the end-of-packet
	TCNT0=47; //104 us overflow   1 bit tim
	TIMSK0|=(1<<TOIE0);//Enable Overflow Interrupt Enable
	
	bus_status.j1708_busy = 1; // the receiver is now busy
	
	uint8_t val;
	val = j1708_rx_limit + 0xFF; // 21 to 0
	
	// if j1708_rx_limit = 0 then (val  = 0xFF and not overflow)
	
	if (val == 0xFF){ // not overflow
		
		j1708_rx_limit = val;
		do_receive();
	}else{ //overflow
		bus_status.j1708_rx_overflow = 1;
	}
	
}

void do_receive(){
	if(bus_status.j1708_active_buffer){
		rx_buffer0();
	}else{
		rx_buffer1();
	}
	
}

void rx_buffer0(){
	j1708_rx_buffer0_count++;
	j1708_rx_buffer0_ptr++;
	rx_buffer_done(); //finish
}

void rx_buffer1(){
	j1708_rx_buffer1_count++;
	j1708_rx_buffer1_ptr++;
	rx_buffer_done(); //finish
}

void rx_buffer_done(){
	
}

void rx_must_check_mid(){
	
	if(bus_status.j1708_rx_check_mid){
		if (j1708_rx_temp == j1708_my_mid){
			//enable Tx interrupt
			UCSR1B |= (1 << UDRIE1); //enable sending
			bus_status.j1708_transmitting = 1;
			bus_status.j1708_rx_check_mid = 0;
		}else{
			rx_collision_detection();
		}
	}else{
		return;
	}
}

void rx_collision_detection(){
	
	j1708_collision_counter ++; // count # collisions
	//random number generate ;
	uint8_t ran;
	ran = j1708_collision_counter & 0x07;
	get_retry_time(ran);
	
	TCNT0=47; //104 us overflow   1 bit tim
	UCSR1B &= ~(1 << UDRIE1); //Disable sending, ready TX buffer

}

void get_retry_time(uint8_t t){
	bit_times = t;
}

void uart1_init(void) {
	UBRR1H = UBRRH_VALUE;
	UBRR1L = UBRRL_VALUE;
	
	send_pos1= 0;
	send_num1= 0;

	#if USE_2X
	UCSR1A |= _BV(U2X1);
	#else
	UCSR1A &= ~(_BV(U2X1));
	#endif

	
	UCSR1B = _BV(RXEN1) | _BV(TXEN1) | _BV(RXCIE1);   /* Enable RX and TX, RX complete ISR */
	UCSR1C = _BV(UCSZ11) | _BV(UCSZ10); /* 8-bit data, 1 stop bit, no parity, asynchronous UART*/

	UCSR1B &= ~(1 << UDRIE1); //Disable sending, ready TX buffer

	//set_sleep_mode(SLEEP_MODE_IDLE);
	//enable global interrupt
	sei();
}


int8_t j1708_send_packet(uint8_t* buffer, uint8_t len){
	
	
	if(bus_status.j1708_tx_busy){
		//fail because we are already sending something
		return -1;
	} else {
		
		j1708_tx_length = len;  // 0 - 19 are valid
		j1708_tx_ptr = 0;
		j1708_tx_buffer = buffer;
		
		j1708_tx_data();
		
		return 0;
	}
	
}

void j1708_tx_data(){
	
	bus_status.j1708_tx_busy = 1; // Now tx is busy
	
	// if the receiver is receiving.. or the idle timer is running... the bus is busy
	
	if(bus_status.j1708_busy || (UCSR1A>>RXC1)&0x01 ){ // we can't bus busy, // unread data in the receive buffer set
		return;		
	}else{
		UDR1 = j1708_my_mid; // send first MID
		j1708_checksum = j1708_my_mid; // start checksum calculation 
		bus_status.j1708_rx_check_mid = 1;
		
		//enable UART interrupt
		UCSR1B |= (1 << UDRIE1); //enable sending
	}
	
}
// send buffer
int8_t uart1_send_buff(uint8_t* buffer, uint8_t num){
	
	if(send_num1){
		//fail because we are already sending something
		return -1;
	} else {
		send_num1= num;
		send_pos1= 0;
		send_buf1= buffer;
		//enable UART interrupt
		UCSR1B |= (1 << UDRIE1); //enable sending

		return 0;
	}
}

int8_t uart1_busy(void){
	if(send_num1 > 0){
		return -1; // busy
	} else {
		return 0; //not busy
	}
}





