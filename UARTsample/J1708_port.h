/* 
* J1708_port.h
*
* Created: 04/08/2017 03:38:15 p.m.
* Author: Freddy
*/


#ifndef __J1708_PORT_H__
#define __J1708_PORT_H__


class J1708_port
{
//variables
public:
	

	static uint8_t* j1708_tx_buffer; 
	static uint8_t j1708_tx_length; 
	static uint8_t j1708_tx_ptr;

	volatile uint8_t* j1708_rx_buffer0;
	volatile uint8_t j1708_rx_buffer0_count;
	volatile uint8_t j1708_rx_buffer0_ptr;

	volatile uint8_t* j1708_rx_buffer1;
	volatile uint8_t j1708_rx_buffer1_count;
	volatile uint8_t j1708_rx_buffer1_ptr;

	volatile uint8_t j1708_checksum;
	volatile uint8_t j1708_rx_temp;
	volatile uint8_t j1708_collision_counter;
	volatile uint8_t j1708_rx_limit;

	volatile uint8_t bit_times = 1;  // values from 1-10
	volatile uint8_t count_times;
	
protected:
private:

	static uint8_t j1708_my_mid = 128; // change this value to change the MID of the module , all modules on the bus should have a different MID

	struct j1708_status {
		volatile uint8_t j1708_rx_check_mid;		//j1708_status,0
		volatile uint8_t j1708_transmitting;		//j1708_status,1
		volatile uint8_t j1708_tx_busy;   			//j1708_status,2
		volatile uint8_t j1708_active_buffer;		//j1708_status,3
		volatile uint8_t j1708_busy;				//j1708_status,4
		volatile uint8_t j1708_checksum_error;		//j1708_status,5
		volatile uint8_t j1708_rx_overflow;			//j1708_status,6
		volatile uint8_t j1708_tx_sent_checksum;	//j1708_status,7
		volatile uint8_t j1708_MID_sent;
	} bus_status;

//functions
public:
	J1708_port();
	~J1708_port();
	
	void j1708_init(void);
	void config_timer0(void);
	
protected:
private:

	void handle_times_isr();
	void j1708_rx_isr_receiving();
	void rx_collision_detection();
	void send_checksum();

	void j1708_send_packet(uint8_t* buffer, uint8_t len); // send bytes, and number of bytes to sent
	void j1708_tx_data();

}; //J1708_port

#endif //__J1708_PORT_H__
