/*
 * uart0.h, USART0 Controlbox RX/TX bus
 *  rx/tx to controlbox or linux serial port
 * Created: 01/08/2017 10:08:11 a.m.
 *  Author: Freddy
 */ 


#ifndef UART0_H_
#define UART0_H_


void uart0_init(void);
int8_t uart0_tx_busy(void);

int8_t uart0_tx_buff(uint8_t* buff_tx, uint8_t len_tx); // send to serial port
int8_t uart0_rx_buff(uint8_t* buff_rx, uint8_t* len_rx); // receive from serial port

void uart0_rx_packet_timeout();

void config_timer2(void);

#endif /* UART0_H_ */