/*
 * uart0.h, USART0 Controlbox RX/TX bus
 *
 * Created: 01/08/2017 10:08:11 a.m.
 *  Author: Freddy
 */ 


#ifndef UART0_H_
#define UART0_H_


void uart0_init(void);
int8_t uart0_send_buff(uint8_t* buffer, uint8_t num);
int8_t uart0_busy(void);


#endif /* UART0_H_ */