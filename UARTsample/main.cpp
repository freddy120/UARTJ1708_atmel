/*
 * UARTsample.cpp
 *
 * Created: 27/07/2017 09:49:08 a.m.
 * Author : Freddy
 */ 

#define F_CPU 16000000UL
#define BUFFER_SIZE 20


#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "uart1.h"
#include "uart0.h"


// buffers for use with the ring buffer (belong to the UART)
uint8_t out_buffer[BUFFER_SIZE];
uint8_t in_buffer[BUFFER_SIZE];


volatile uint8_t buff[30];

uint8_t  test_string[] = "Hello\n";
volatile bool flag_rx = false;




void uart_putchar(char c) {
	loop_until_bit_is_set(UCSR1A, UDRE1); /* Wait until data register empty. */
	UDR1 = c;
	loop_until_bit_is_set(UCSR1A, TXC1); /* Wait until transmission ready. */
	
}

char uart_getchar(void) {
	loop_until_bit_is_set(UCSR1A, RXC1); /* Wait until data exists. */
	return UDR1;
}

void putString (char *str)
{
	while (*str) 
		uart_putchar(*str++);
}







int main(void)
{
	uart1_init();
	uart0_init();
	
	
    /* Replace with your application code */
    while (1) 
    {
		if(flag_rx){
			int8_t t;
			//uart1_send_buff(test_string,6);//strlen(test_string));

			flag_rx = false;
		}
		//int i;
		////putString("Hello world!\n");
		//for(i=0;i<1000;i++){//delay 1s
			//_delay_ms(1);
		//}
		
    }
}





