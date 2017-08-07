/*
 * j1708.h
 *
 * Created: 07/08/2017 03:19:11 p.m.
 *  Author: Freddy
 */ 


#ifndef J1708_H_
#define J1708_H_


void j1708_init(void);
void config_timer0(void);
void handle_times_isr();
void j1708_rx_isr_receiving();
void rx_collision_detection();
void send_checksum();

void j1708_send_packet(uint8_t* buffer, uint8_t len); // send bytes, and number of bytes to sent
void j1708_tx_data();
int8_t j1708_read_buffer(uint8_t* buffer, uint8_t* len); // red buffer and len of buffer


#endif /* J1708_H_ */