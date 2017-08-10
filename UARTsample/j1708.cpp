/*
 * j1708.cpp
 *
 * Created: 07/08/2017 03:19:28 p.m.
 *  Author: Freddy
 */ 


#define F_CPU 16000000UL  // 16 MHz
#define BAUD 9600
#include <util/setbaud.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include <stdio.h>
#include <stdint.h>

#include "j1708.h"

#define j1708_my_mid 128 // change this value to change the MID of the module , all modules on the bus should have a different MID
#define j1708_priority 8 // change priority of my packets 1-8, 7-8 for all other messages

#define BUFFER_IN_SIZE 30
#define BIT_TIMES_10 11


static uint8_t* j1708_tx_buffer;
volatile uint8_t j1708_tx_length;
volatile uint8_t j1708_tx_ptr;

volatile uint8_t j1708_rx_buffer0[BUFFER_IN_SIZE];
volatile uint8_t j1708_rx_buffer0_count;
volatile uint8_t j1708_rx_buffer0_ptr;

//volatile uint8_t j1708_rx_buffer1[BUFFER_IN_SIZE];
//volatile uint8_t j1708_rx_buffer1_count;
//volatile uint8_t j1708_rx_buffer1_ptr;

volatile uint8_t j1708_rx_buff_save[BUFFER_IN_SIZE];
volatile uint8_t len_rx_save;



volatile uint8_t j1708_checksum;
volatile uint8_t j1708_rx_temp;
volatile uint8_t j1708_collision_counter;
volatile uint8_t j1708_rx_limit; //BUFFER_IN_SIZE

volatile uint8_t bit_times = 0;  // values from 1-10
volatile uint8_t count_times;


struct j1708_status {
	
	volatile uint8_t j1708_transmitting;		
	volatile uint8_t j1708_tx_busy;   			
	volatile uint8_t j1708_active_buffer;		
	volatile uint8_t j1708_busy;				
	volatile uint8_t j1708_checksum_error;				
	volatile uint8_t j1708_tx_sent_checksum;	
	volatile uint8_t j1708_MID_sent;
	volatile uint8_t j1708_random_collision_times;
	
	volatile uint8_t j1708_wait_idle_time;
	volatile uint8_t j1708_priority_check_flag;
	volatile uint8_t j1708_finish_read_packet;

} bus_status;




void config_timer0(void){
	// Prescaler = FCPU/8
	TCCR0A |= (1<<CS01);//(1<<CS02) | (1<<CS00);
	//sei();	
	TIMSK0 &= ~(1<<TOIE0);
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
		handle_times_isr(); // then reach 10 bit time idle time
	}
}

//used to generate an interrupt on the end of a packet AND as a transmit collision retry timer
void handle_times_isr(){
	
	bus_status.j1708_busy = 0; // not busy idle time, 10 bit times
	
	if(bus_status.j1708_random_collision_times){// wait pseudo random bit times
		bus_status.j1708_random_collision_times = 0;
		j1708_collision_counter = 0; // reset collision counter
		j1708_tx_data();
		return;
	}
		
	if(bus_status.j1708_priority_check_flag){// reach priority delay
		bus_status.j1708_priority_check_flag = 0;
		j1708_tx_data();
		return;
	}
	
	if(bus_status.j1708_transmitting){ // when are transmitting data bytes
		// read nothing
	}else{  // receive packet, end of packet and I can try tx
		// checksum
		if(j1708_checksum==0){ //valid
			bus_status.j1708_checksum_error = 0;
		}else{//invalid
			bus_status.j1708_checksum_error = 1;
		}
		
		// save buffer;
		int i;
		for(i=0;i<j1708_rx_buffer0_count;i++){
			j1708_rx_buff_save[i] = j1708_rx_buffer0[i];
		}
		len_rx_save = j1708_rx_buffer0_count;
		
		if(len_rx_save!=0){ // we have a read packet
			bus_status.j1708_finish_read_packet = 1;
		}
		
		j1708_rx_buffer0_ptr = 0;
		j1708_rx_buffer0_count = 0;
		
		if (bus_status.j1708_tx_busy){
			//// need to add priority time 
			count_times = 0;
			bit_times = j1708_priority; //priority delay
			TCNT0=47; //104 us overflow   1 bit tim
			TIMSK0|=(1<<TOIE0);//Enable Overflow Interrupt Enable
			bus_status.j1708_priority_check_flag = 1;
		}
		
		
		//if(bus_status.j1708_active_buffer){// change buffer active 
			//
			//bus_status.j1708_active_buffer = 0;
			//j1708_rx_buffer0_ptr = 0;
			//j1708_rx_buffer0_count = 0;
			//if (bus_status.j1708_tx_busy){
				//// need to add priority time + recheck time
				////j1708_tx_data(); // try again
				//count_times = 0;
				//bit_times = j1708_priority; //priority delay
				//TCNT0=47; //104 us overflow   1 bit tim
				//TIMSK0|=(1<<TOIE0);//Enable Overflow Interrupt Enable
				//bus_status.j1708_priority_check_flag = 1;
			//}
				//
		//}else{
				//
			//bus_status.j1708_active_buffer = 1;
			//j1708_rx_buffer1_ptr = 0;
			//j1708_rx_buffer1_count = 0;
			//if (bus_status.j1708_tx_busy){
				////j1708_tx_data();
				//count_times = 0;
				//bit_times = j1708_priority; //priority delay
				//TCNT0=47; //104 us overflow   1 bit tim
				//TIMSK0|=(1<<TOIE0);//Enable Overflow Interrupt Enable
				//bus_status.j1708_priority_check_flag = 1;
			//}
				//
		//}
		
		
	}
}


/**
 * \brief UART data register empty interrupt handler
 *
 * This handler is called each time the UART data register is available for
 * sending data.
 */
ISR(USART1_UDRE_vect)
{
	if(j1708_tx_length > j1708_tx_ptr){
		UDR1 = j1708_tx_buffer[j1708_tx_ptr];
		j1708_checksum += j1708_tx_buffer[j1708_tx_ptr]; // add to checksum
		j1708_tx_ptr++;
	} else {
		j1708_tx_length = 0;
		//disable sending interrupt
		UCSR1B &= ~(1 << UDRIE1); //Disable sending
		
		send_checksum();
	}
	
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

	
	bus_status.j1708_priority_check_flag = 0; // clear priority check
	bus_status.j1708_random_collision_times = 0;//clear collision flag
	j1708_collision_counter = 0; // reset collision counter
	
	// transmitting indicate idle bus has reached end we are tx MID+data
	if (bus_status.j1708_transmitting){ //  yes, so we must..
		
		if(bus_status.j1708_tx_sent_checksum){
			bus_status.j1708_transmitting = 0; // clear flag tx
			bus_status.j1708_tx_sent_checksum = 0; // clear flag checksum
			
			bus_status.j1708_wait_idle_time = 0;
		}
		
	}else{ // no, so go receive this data
		if(bus_status.j1708_MID_sent==0){
			j1708_rx_isr_receiving();
		}
	}
	
	if(bus_status.j1708_tx_busy && bus_status.j1708_MID_sent){ // MID was sent for j1708_tx_data()
		//need to check if is correct
		if(j1708_rx_temp == j1708_my_mid){ // then we are trasnmitting, check if MID is the same
			bus_status.j1708_transmitting = 1;
			bus_status.j1708_MID_sent = 0; 
			UCSR1B |= (1 << UDRIE1); //enable Tx interrupt, enable sending
		}else{
			bus_status.j1708_MID_sent = 0; 
			rx_collision_detection();
		}
	}

}

void j1708_rx_isr_receiving(){
	
	
	bus_status.j1708_busy = 1; // the receiver is now busy

	// reset end idle time
	count_times = 0;
	bit_times = BIT_TIMES_10;//10; //reset the end-of-packet
	TCNT0=47; //104 us overflow   1 bit tim
	TIMSK0|=(1<<TOIE0);//Enable Overflow Interrupt Enable
	
	
	// no limit of bytes to received
	j1708_rx_buffer0[j1708_rx_buffer0_ptr] = j1708_rx_temp;
	j1708_rx_buffer0_count++; //count number of bytes on buffer
	j1708_rx_buffer0_ptr++;
	// add to checksum
	j1708_checksum += j1708_rx_temp;

	//if (bus_status.j1708_active_buffer){ // so use buffer1
		//
		//// no limit of bytes to received
		//j1708_rx_buffer1[j1708_rx_buffer1_ptr] = j1708_rx_temp;
		//j1708_rx_buffer1_count++; //count number of bytes on buffer
		//j1708_rx_buffer1_ptr++;
		//// add to checksum
		//j1708_checksum += j1708_rx_temp;
		////limit of bytes
		//if(j1708_rx_buffer1_count == 21){// max 21 bytes
			//bus_status.j1708_rx_overflow = 1;
		//}
		//
		//
	//}else{ // so use buffer0
		//// no limit of bytes to received
		//j1708_rx_buffer0[j1708_rx_buffer0_ptr] = j1708_rx_temp;
		//j1708_rx_buffer0_count++; //count number of bytes on buffer
		//j1708_rx_buffer0_ptr++;
		//// add to checksum
		//j1708_checksum += j1708_rx_temp;
		////limit of bytes
		//if(j1708_rx_buffer0_count == 21){// max 21 bytes
			//bus_status.j1708_rx_overflow = 1;
		//}
		//
	//}

}




void send_checksum(){
	uint8_t cs;
	cs = ~(j1708_checksum);
	cs += 1;
	UDR1 = cs; // send checksum
	
	//UCSR1B &= ~(1 << UDRIE1);  //disable TX isr 
	//UCSR1B &= ~(1 << RXEN1);  //enable RX isr
	
	bus_status.j1708_tx_busy = 0; // tx queue is now empty
	bus_status.j1708_tx_sent_checksum = 1;
	
}



void rx_collision_detection(){
	
	j1708_collision_counter ++; // count # collisions
	
	if(j1708_collision_counter==1){//first collision then wait 10 bit times again + priority bit times
		bit_times = BIT_TIMES_10;//10;
		count_times = 0;
		TCNT0=47; //104 us overflow   1 bit time
		TIMSK0|=(1<<TOIE0);//Enable Overflow Interrupt Enable
	}else{
		//random number generate ;
		bit_times = j1708_collision_counter & 0x07; //random time from 1 - 7
		count_times = 0;
		TCNT0=47; //104 us overflow   1 bit time
		TIMSK0|=(1<<TOIE0);//Enable Overflow Interrupt Enable
		
		bus_status.j1708_random_collision_times = 1;
	}
	
}



void j1708_init(void) { // init uart1, rx interrupt enable and tx interrupt disable
	
	j1708_tx_ptr = 0;
	j1708_tx_length = 0;
	j1708_rx_buffer0_ptr = 0;
	j1708_rx_buffer0_count = 0;
	j1708_checksum = 0;
	j1708_rx_temp = 0;
	j1708_collision_counter = 0;
	
	bus_status.j1708_busy = 0;
	bus_status.j1708_tx_busy = 0;
	bus_status.j1708_MID_sent = 0;
	bus_status.j1708_finish_read_packet = 0;
	bus_status.j1708_priority_check_flag = 0;
	bus_status.j1708_random_collision_times = 0;
	bus_status.j1708_transmitting = 0;
	bus_status.j1708_wait_idle_time = 0;
	bus_status.j1708_tx_sent_checksum = 0;
	
	//init uart1 
	UBRR1H = UBRRH_VALUE;
	UBRR1L = UBRRL_VALUE;

	#if USE_2X
	UCSR1A |= _BV(U2X1);
	#else
	UCSR1A &= ~(_BV(U2X1));
	#endif

	
	UCSR1B = _BV(RXEN1) | _BV(TXEN1) | _BV(RXCIE1);   /* Enable RX and TX, RX complete ISR */
	UCSR1C = _BV(UCSZ11) | _BV(UCSZ10); /* 8-bit data, 1 stop bit, no parity, asynchronous UART*/

	UCSR1B &= ~(1 << UDRIE1); //Disable sending, ready TX buffer


	config_timer0();
	//enable global interrupt
	sei();
	
}

/*
*	j1708_send_packet
*	load data into j1708_tx_buffer, and length of data not include MID byte, scheduled
*/
void j1708_send_packet(uint8_t* buffer, uint8_t len){ // load data into j1708_tx_buffer
	
	if(bus_status.j1708_tx_busy == 0){ // not busy tx 
		
		j1708_tx_length = len;
		j1708_tx_ptr = 0;
		j1708_tx_buffer = buffer;
		
		if ((bus_status.j1708_wait_idle_time==0) && (bus_status.j1708_busy==0)){
			
			count_times = 0;
			bit_times = BIT_TIMES_10;//10; //10 bit times reset the end-of-packet
			TCNT0=47; //104 us overflow   1 bit times
			TIMSK0|=(1<<TOIE0);//Enable Overflow Interrupt Enable
			
			bus_status.j1708_wait_idle_time = 1;
		}
		
		bus_status.j1708_tx_busy = 1; // we are busy now
	}
		
}

/*
	try to send data packet, if failed just return without sending nothing
*/
void j1708_tx_data(){
	
	//bus_status.j1708_tx_busy = 1; // Now tx is busy
	
	// if the receiver is receiving.. or the idle timer is running... the bus is busy
	if(bus_status.j1708_busy){ //|| (UCSR1A>>RXC1)&0x01 ){ // we can't bus busy, // unread data in the receive buffer set
		return;
	}else{
		UDR1 = j1708_my_mid; // send first MID
		j1708_checksum = j1708_my_mid; // start checksum calculation 		
		bus_status.j1708_MID_sent = 1;
	}
	
}


int8_t j1708_read_buffer(uint8_t* buffer, uint8_t* len){

	if(bus_status.j1708_finish_read_packet){
		uint8_t i;
		for(i=0;i<len_rx_save;i++){
			buffer[i] = j1708_rx_buff_save[i];
		}
		*len = len_rx_save;
		bus_status.j1708_finish_read_packet = 0;
		return 0;
	}else{
		return -1;
	}
}



