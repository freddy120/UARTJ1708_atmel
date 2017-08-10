/*
 * main.cpp
 * J1708 interface, reading and writing on J1708 bus
 * Created: 27/07/2017 09:49:08 a.m.
 * Author : Freddy
 */ 

#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>


#define BUFFER_IN_SIZE 30


#include "uart0.h"
#include "j1708.h"

//#define BUFFER_IN_SIZE 30

// buffers j1708 rx and tx
uint8_t j1708_in_buffer[BUFFER_IN_SIZE];
uint8_t j1708_in_len;

//buffers uart0 for PC com
uint8_t uart0_in_buffer[BUFFER_IN_SIZE];
uint8_t uart0_in_len;



// main
int main(void)
{
	
	uart0_init();
	j1708_init();

	
	int8_t res_rx_uart0 = -1;
	int8_t res_rx_j1708 = -1;
    /* Replace with your application code */
    while (1) 
    {
		res_rx_uart0 = uart0_rx_buff(uart0_in_buffer,&uart0_in_len);
		res_rx_j1708 = j1708_read_buffer(j1708_in_buffer,&j1708_in_len);
		if(res_rx_uart0 == 0){
			
			j1708_send_packet(uart0_in_buffer,uart0_in_len); // shedule send packet to j1708 bus;
		}
		
		if (res_rx_j1708 == 0){
			j1708_in_buffer[j1708_in_buffer] = 'M';
			j1708_in_buffer[j1708_in_buffer+1] = 'S';
			j1708_in_buffer[j1708_in_buffer+2] = 'S';
			
			uart0_tx_buff(j1708_in_buffer,j1708_in_len+3);  // send to uart0, need to add  'MSS' at the end
			
		}

		
		
    }
}





