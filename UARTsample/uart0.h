/*
 * uart0.h, USART0 Controlbox RX/TX bus
 *  rx/tx to controlbox or linux serial port
 * Created: 01/08/2017 10:08:11 a.m.
 *  Author: Freddy
 */ 


#ifndef UART0_H_
#define UART0_H_


void uart0_init(void);
int8_t uart0_send_buff(uint8_t* buffer, uint8_t num);
int8_t uart0_busy(void);


int8_t uart0_tx_buff(uint8_t* buff_tx, uint8_t len_tx);
void uart0_rx_buff(uint8_t* buff_rx, uint8_t* len_rx);


#endif /* UART0_H_ */