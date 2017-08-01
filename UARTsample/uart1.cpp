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
	
	//UDR1=temp; //tx byte
	
	UDR1 = temp;
	
	//flag_rx = true;
	
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





