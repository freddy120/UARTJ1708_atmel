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

uint8_t j1708_tx_buffer[20];
uint8_t j1708_rx_buffer0[21];
uint8_t j1708_rx_buffer1[21];
uint8_t j1708_rx_buffer0_count;
uint8_t j1708_rx_buffer1_count;

uint8_t j1708_tx_ptr;
uint8_t j1708_tx_length;
uint8_t j1708_rx_buffer0_ptr;
uint8_t j1708_rx_buffer1_ptr;
uint8_t j1708_checksum;
volatile uint8_t j1708_status;
volatile uint8_t j1708_rx_temp;
uint8_t j1708_collision_counter;
uint8_t j1708_rx_limit;

volatile uint8_t bit_times = 1;  // values from 1-10
volatile uint8_t count_times;


#define j1708_rx_check_mid		0 //j1708_status,0
#define j1708_transmitting		1 //j1708_status,1
#define j1708_tx_busy   		2 //j1708_status,2
#define j1708_active_buffer		3 //j1708_status,3
#define j1708_busy				4 //j1708_status,4
#define j1708_checksum_error	5 //j1708_status,5
#define j1708_rx_overflow		6 //j1708_status,6
#define j1708_tx_sent_checksum	7 //j1708_status,7
//TIMER0

volatile bool flag_count = false;

void config_timer0(void){
	// Prescaler = FCPU/8
	TCCR0A |= (1<<CS01);//(1<<CS02) | (1<<CS00);

	
	//Enable Overflow Interrupt Enable
	TIMSK0|=(1<<TOIE0);
	
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
	if(send_num1 > send_pos1){
		UDR1= send_buf1[send_pos1];
		send_pos1++;
	} else {
		send_num1= 0;
		//disable sending interrupt
		UCSR1B &= ~(1 << UDRIE1); //Disable sending
	}
}

/**
 * \brief Data RX interrupt handler
 *
 * This is the handler for UART receive data
 */
ISR(USART1_RX_vect)
{
	//ring_buffer_put(&ring_buffer_in, UDR0);
	uint8_t temp;
	temp = UDR1; // read buffer
	
	j1708_rx_temp = temp; // save on temp
	
	if (j1708_status == j1708_transmitting){
		
		
	}else{
		j1708_rx_isr_receiving();
		
	}
	//UDR1=temp; //tx byte
	
	//UDR1 = temp;
	
	//flag_rx = true;
	
}

void j1708_rx_isr_receiving(){
	
	bit_times = 10; //reset the end-of-packet
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





