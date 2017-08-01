/*
 * uart0.cpp
 *
 * Created: 01/08/2017 10:08:00 a.m.
 *  Author: Freddy
 */ 


#define F_CPU 16000000UL  // 16 MHz
#define BAUD 9600
#include <util/setbaud.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include <stdio.h>
#include <stdint.h>

#include "uart0.h"


//Variables for USART1, J1708 bus.
static uint8_t send_pos0;
static uint8_t send_num0;
static uint8_t* send_buf0;


/**
 * \brief UART data register empty interrupt handler
 *
 * This handler is called each time the UART data register is available for
 * sending data.
 */
ISR(USART0_UDRE_vect)
{
	if(send_num0 > send_pos0){
		UDR0= send_buf0[send_pos0];
		send_pos0++;
	} else {
		send_num0= 0;
		//disable sending interrupt
		UCSR0B &= ~(1 << UDRIE0); //Disable sending
	}
}

/**
 * \brief Data RX interrupt handler
 *
 * This is the handler for UART receive data
 */
ISR(USART0_RX_vect)
{
	//ring_buffer_put(&ring_buffer_in, UDR0);
	uint8_t temp;
	
	
	temp = UDR0; // read buffer
	
	//UDR1=temp; //tx byte
	
	UDR0 = temp;
	
	//flag_rx = true;
	
}


void uart0_init(void) {
	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;
	
	send_pos0= 0;
	send_num0= 0;

	#if USE_2X
	UCSR0A |= _BV(U2X0);
	#else
	UCSR0A &= ~(_BV(U2X0));
	#endif

	
	UCSR0B = _BV(RXEN0) | _BV(TXEN0) | _BV(RXCIE0);   /* Enable RX and TX, RX complete ISR */
	UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); /* 8-bit data, 1 stop bit, no parity, asynchronous UART*/

	UCSR0B &= ~(1 << UDRIE0); //Disable sending, ready TX buffer

	//set_sleep_mode(SLEEP_MODE_IDLE);
	//enable global interrupt
	sei();
}

// send buffer
int8_t uart0_send_buff(uint8_t* buffer, uint8_t num){
	
	if(send_num0){
		//fail because we are already sending something
		return -1;
	} else {
		send_num0 = num;
		send_pos0 = 0;
		send_buf0 = buffer;
		//enable UART interrupt
		UCSR0B |= (1 << UDRIE0); //enable sending

		return 0;
	}
}

int8_t uart0_busy(void){
	if(send_num0 > 0){
		return -1; // busy
	} else {
		return 0; //not busy
	}
}



