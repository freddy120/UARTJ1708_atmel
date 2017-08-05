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
static uint8_t* j1708_in_buffer;

//buffers uart0 for PC com
static uint8_t* uart0_out_buffer;
static uint8_t* uart0_in_buffer;


//TIMER2
volatile uint32_t count_tmr2;
volatile bool flag_tmr2 = false;


// config timer2 
void config_timer2(void){
	// Pre scaler = FCPU/8
	TCCR2A |= (1<<CS21);
	//Enable Overflow Interrupt Enable
	TIMSK2|=(1<<TOIE2);
	//enable global interrupt
	sei();
	//Initialize Counter
	TCNT2=47; //104 us overflow   1 bit time
}

// timer2 ISR
ISR(TIMER2_OVF_vect)
{
	TCNT2=47; //104 us overflow   1 bit time
	
	count_tmr2++;
	if(count_tmr2==7843) //1 second
	{
		flag_tmr2 = true;
		count_tmr2=0;
	}
}

// main
int main(void)
{
	config_timer2();
	j1708_init();
	uart0_init();
	
	
    /* Replace with your application code */
    while (1) 
    {
		if(flag_tmr2){
			uart0_send_buff( (uint8_t*)"hello\n",6); //testing
			flag_tmr2 = false;
		}
		
		
    }
}





