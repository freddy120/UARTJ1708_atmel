/*
 * j1708.h
 *
 * Created: 07/08/2017 03:19:11 p.m.
 *  Author: Freddy
 */ 


#ifndef J1708_H_
#define J1708_H_

void uart1_init(void);
int8_t uart1_tx_busy(void);

int8_t uart1_tx_buff(uint8_t* buff_tx, uint8_t len_tx); // send to serial port
int8_t uart1_rx_buff(uint8_t* buff_rx, uint8_t* len_rx); // receive from serial port

void uart1_rx_packet_timeout();

void config_timer0_2(void);



#endif /* J1708_H_ */