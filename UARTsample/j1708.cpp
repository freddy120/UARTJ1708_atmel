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

#define BUFFER_IN_SIZE 30

volatile uint8_t uart1_ptr_out;
volatile uint8_t uart1_len_out;
volatile  uint8_t* uart1_buffer_out; 

volatile uint8_t uart1_buffer_in[BUFFER_IN_SIZE];
volatile uint8_t uart1_ptr_in;
volatile uint8_t uart1_count_in;

volatile uint8_t  uart1_rx_save[BUFFER_IN_SIZE];
volatile uint8_t uart1_rx_len_save;

volatile uint8_t flag_finish_rx_packet_2 = false;

//TIMER2
volatile uint8_t count_tmr0_2;

/**
 * \brief UART data register empty interrupt handler
 *
 * This handler is called each time the UART data register is available for
 * sending data.
 */
ISR(USART1_UDRE_vect)
{
	if(uart1_len_out > uart1_ptr_out){
		UDR1= uart1_buffer_out[uart1_ptr_out];
		uart1_ptr_out++;
	}else{
		uart1_len_out = 0; // reset
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
	
	uart1_buffer_in[uart1_ptr_in] = temp;
	uart1_ptr_in++;

	TCNT0 = 47;
	count_tmr0_2 = 0;
	TIMSK0 |= (1<<TOIE0); // enable timer isr
	
}



// timer2 ISR
ISR(TIMER0_OVF_vect)
{
	TCNT0=47; //104 us overflow   1 bit time
	
	count_tmr0_2++;
	if(count_tmr0_2==10) // when reach 1ms, timeout 1ms
	{
		count_tmr0_2=0;
		uart1_rx_packet_timeout();

	}
}

void uart1_rx_packet_timeout(){
	flag_finish_rx_packet_2 = 1; // clear when read rx buffer
	
	TIMSK0 &= ~(1<<TOIE0); // disable timer isr
	int i;
	for(i=0;i<uart1_ptr_in;i++){
		uart1_rx_save[i] = uart1_buffer_in[i];
	}
	
	uart1_count_in = uart1_ptr_in; // save count rx bytes
	uart1_rx_len_save = uart1_count_in;
	uart1_ptr_in = 0; // reset ptr in

}


void uart1_init(void) {
	UBRR1H = UBRRH_VALUE;
	UBRR1L = UBRRL_VALUE;
	
	uart1_len_out = 0;
	uart1_ptr_out = 0;

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
	config_timer0_2();
	sei();

}


int8_t uart1_tx_busy(void){
	if(uart1_len_out > 0){
		return -1; // busy
	} else {
		return 0; //not busy
	}
}



int8_t uart1_tx_buff(uint8_t* buff_tx, uint8_t len_tx){ // send to serial port
	if(uart1_len_out){
		//fail because we are already sending something
		return -1;
	} else {
		uart1_len_out = len_tx;
		uart1_ptr_out = 0;
		uart1_buffer_out = buff_tx;
		//enable UART interrupt
		UCSR1B |= (1 << UDRIE1); //enable sending

		return 0;
	}
}


int8_t uart1_rx_buff(uint8_t* buff_rx, uint8_t* len_rx){ // receive from serial port
	if(flag_finish_rx_packet_2){ //packet complete
		flag_finish_rx_packet_2 = 0; // just clear flag
		int i;
		for(i=0;i<uart1_rx_len_save;i++){
			buff_rx[i]=uart1_rx_save[i]; //transfer buffer
		}
		*len_rx = uart1_rx_len_save; // transfer len of buffer
		return 0;
	}else{ // cant read buff try again later;
		return -1;
	}
}


// config timer2
void config_timer0_2(void){
	// Pre scaler = FCPU/8
	TCCR0A |= (1<<CS01);
	//Enable Overflow Interrupt Enable
	//TIMSK2|=(1<<TOIE2);
	
	//Initialize Counter
	TCNT0=47; //104 us overflow   1 bit time
}