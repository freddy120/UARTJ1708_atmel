/*
 * uart1.h, for J1708 bus tx1 rx1
 *
 * Created: 01/08/2017 10:07:31 a.m.
 *  Author: Freddy
 */ 


#ifndef UART1_H_
#define UART1_H_

void uart1_init(void);
int8_t uart1_send_buff(uint8_t* buffer, uint8_t num);
int8_t uart1_busy(void);



#endif /* UART1_H_ */