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

#define BUFFER_IN_SIZE 30

volatile uint8_t uart0_ptr_out;
volatile uint8_t uart0_len_out;
volatile  uint8_t* uart0_buffer_out; 

volatile uint8_t uart0_buffer_in[BUFFER_IN_SIZE];
volatile uint8_t uart0_ptr_in;
volatile uint8_t uart0_count_in;

volatile uint8_t  uart0_rx_save[BUFFER_IN_SIZE];
volatile uint8_t uart0_rx_len_save;

volatile uint8_t flag_finish_rx_packet = 0;

//TIMER2
volatile uint8_t count_tmr2;

/**
 * \brief UART data register empty interrupt handler
 *
 * This handler is called each time the UART data register is available for
 * sending data.
 */
ISR(USART0_UDRE_vect)
{
	if(uart0_len_out > uart0_ptr_out){
		UDR0= uart0_buffer_out[uart0_ptr_out];
		uart0_ptr_out++;
		if(uart0_ptr_out==uart0_len_out){
			uart0_len_out = 0;
			////disable sending interrupt
			UCSR0B &= ~(1 << UDRIE0); //Disable sending
		}
		
	}
	//} else {
		//uart0_len_out = 0; // reset
		////disable sending interrupt
		//UCSR0B &= ~(1 << UDRIE0); //Disable sending
	//}
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
	
	uart0_buffer_in[uart0_ptr_in] = temp;
	uart0_ptr_in++;

	TCNT2 = 47;
	count_tmr2 = 0;
	TIMSK2 |= (1<<TOIE2); // enable timer isr
	
}



// timer2 ISR
ISR(TIMER2_OVF_vect)
{
	TCNT2=47; //104 us overflow   1 bit time
	
	count_tmr2++;
	if(count_tmr2==10) // when reach 1ms, timeout 1ms
	{
		count_tmr2=0;
		TIMSK2 &= ~(1<<TOIE2); // disable timer isr
		uart0_rx_packet_timeout();

	}
}

void uart0_rx_packet_timeout(){
	
	uint8_t i;
	for(i=0;i<uart0_ptr_in;i++){
		uart0_rx_save[i] = uart0_buffer_in[i];
	}
	
	uart0_count_in = uart0_ptr_in; // save count rx bytes
	uart0_rx_len_save = uart0_count_in;
	uart0_ptr_in = 0; // reset ptr in
	
	flag_finish_rx_packet = 1; // clear when read rx buffer

}


void uart0_init(void) {
	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;
	
	uart0_len_out = 0;
	uart0_ptr_out = 0;
	uart0_rx_len_save = 0;

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
	config_timer2();
	sei();

}


int8_t uart0_tx_busy(void){
	if(uart0_len_out > 0){
		return -1; // busy
	} else {
		return 0; //not busy
	}
}



int8_t uart0_tx_buff(uint8_t* buff_tx, uint8_t len_tx){ // send to serial port
	if(uart0_len_out){
		//fail because we are already sending something
		return -1;
	} else {
		uart0_len_out = len_tx;
		uart0_ptr_out = 0;
		uart0_buffer_out = buff_tx;
		//enable UART interrupt
		UCSR0B |= (1 << UDRIE0); //enable sending

		return 0;
	}
}


int8_t uart0_rx_buff(uint8_t* buff_rx, uint8_t* len_rx){ // receive from serial port
	if(flag_finish_rx_packet){ //packet complete
		flag_finish_rx_packet = 0; // just clear flag
		uint8_t kk;
		for(kk=0;kk<uart0_rx_len_save;kk++){
			buff_rx[kk]=uart0_rx_save[kk]; //transfer buffer
		}
		*len_rx = uart0_rx_len_save; // transfer len of buffer
		return 0;
	}else{ // cant read buff try again later;
		return -1;
	}
}


// config timer2
void config_timer2(void){
	// Pre scaler = FCPU/8
	TCCR2A |= (1<<CS21);
	//Enable Overflow Interrupt Enable
	//TIMSK2|=(1<<TOIE2);
	
	//Initialize Counter
	TCNT2=47; //104 us overflow   1 bit time
}