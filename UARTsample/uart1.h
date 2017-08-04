/*
 * uart1.h, for J1708 bus tx1 rx1
 *
 * Created: 01/08/2017 10:07:31 a.m.
 *  Author: Freddy
 */ 


#ifndef UART1_H_
#define UART1_H_


void j1708_init(void);
void config_timer0(void);
void handle_times_isr();
void j1708_rx_isr_receiving();
void rx_collision_detection();
void send_checksum();

void j1708_send_packet(uint8_t* buffer, uint8_t len); // send bytes, and number of bytes to sent
void j1708_tx_data();


#endif /* UART1_H_ */