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

#include "uart1.h"
#include "uart0.h"

// buffers j1708 rx and tx
static uint8_t* j1708_out_buffer;
static uint8_t j1708_out_len;
static uint8_t* j1708_in_buffer;
static uint8_t j1708_in_len;

//buffers uart0 for PC com
static uint8_t* uart0_out_buffer;
static uint8_t uart0_out_len;
static uint8_t* uart0_in_buffer;
static uint8_t uart0_in_len;



// main
int main(void)
{
	j1708_init();
	uart0_init();
	
	int8_t res_rx_uart0;
	int8_t res_rx_j1708;
    /* Replace with your application code */
    while (1) 
    {
		res_rx_uart0 = uart0_rx_buff(uart0_in_buffer,&uart0_in_len);
		res_rx_j1708 = j1708_read_buffer(j1708_in_buffer,&j1708_in_len);
	  
		if(res_rx_uart0 < 0){
			//error we dont have a complete rx packet yet, try later
		}else{
			j1708_send_packet(uart0_in_buffer,uart0_in_len); // shedule send packet to j1708 bus;
		}

		if (res_rx_j1708 < 0){
			//error we dont have a complete rx packet from j1708 bus yet
		}else{
			 uart0_tx_buff(j1708_in_buffer, j1708_in_len); //send to serial port
		}
		
    }
}





