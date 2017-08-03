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
void config_timer0(void);


void j1708_rx_isr_receiving();
void rx_must_check_mid();
void rx_collision_detection();

int8_t j1708_send_packet(uint8_t* buffer, uint8_t len); // send bytes, and number of bytes to sent
void j1708_tx_data();
void handle_buffer0();
void handle_buffer1();


#endif /* UART1_H_ *