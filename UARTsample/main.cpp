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

#include "uart0.h"
#include "j1708.h"

#define BUFFER_IN_SIZE 30

// buffers j1708 rx and tx
uint8_t j1708_in_buffer[BUFFER_IN_SIZE];
uint8_t j1708_in_len;

//buffers uart0 for PC com
uint8_t uart0_in_buffer[BUFFER_IN_SIZE];
uint8_t uart0_in_len;

static uint8_t packet1[] = "ABCD";


// main
int main(void)
{
	j1708_init();
	uart0_init();

	
	int8_t res_rx_uart0;
	//int8_t res_rx_j1708;
    /* Replace with your application code */
    while (1) 
    {
		res_rx_uart0 = uart0_rx_buff(uart0_in_buffer,&uart0_in_len);
		//res_rx_j1708 = j1708_read_buffer(j1708_in_buffer,&j1708_in_len);
	  
		if(res_rx_uart0==0){
			j1708_send_packet(packet1,4); // shedule send packet to j1708 bus;
			//j1708_send_packet(packet1,6); // shedule send packet to j1708 bus;
			//j1708_send_packet(uart0_in_len,1);
		}

		//if (res_rx_j1708 < 0){
			//error we dont have a complete rx packet from j1708 bus yet
		//}else{
		//	uart0_tx_buff(j1708_in_buffer,j1708_in_len); //send to serial port
		//}
		
    }
}





