/*
 * wireless.c
 *
 * Created: 2/26/2012
 * Re-factored 4/24/12
 *  Author: Evan
 */
 
/***************
Conventions/notes:
-Functions and variables pertaining to the communication portion of the code are prefixed by
 either tx_ or rx_
-Functions that fulfill a similar purpose for RX and TX modes are named symmetrically where possible
-Packet length = head + data + termination character
-Telemetry functions (that access the TX buffer through tx_copy2buff) will fail if buffer is in use.
 This must be handled explicitly for critical packets.
****************/

#include "wireless.h"

char rx_buff[RX_BUFF_SIZE];	//receive buffer for incoming packets
char tx_buff[TX_BUFF_SIZE];	//transmit buffer for outgoing packets
char tmp_buff[RX_MAX_SIZE]; //intermediate buffer between rx_postman and logistics

int diag;			//flag for extended telemetry stuff
int rx_cur_loc;
int rx_end;
int rx_buff_count;	//Number of received bytes in incomplete packet
int tx_cur_loc;
int tx_end;
int tx_buff_count;
long RTC_last;		//Real-time clock 
int usart_hit;

//rx interrupt service routine
__attribute__((__interrupt__)) static void wifi_usart_rx_isr(void)
{
	//(WI_USART)->csr;
	rx_buff[rx_end] = WI_USART->rhr & 0xFF;
	rx_buff_count++;
	rx_end++;
	if(rx_end==RX_BUFF_SIZE) rx_end-=RX_BUFF_SIZE;
}//end wifi_usart_rx_isr

//sets the memory locations of needed structs
void wireless_init(void){
	diag = 0; //send extended information flag
	rx_buff_count = 0;
	rx_cur_loc = 0;
	rx_end = 0;
	tx_cur_loc = 0;
	tx_end = 0;
	tx_buff_count = 0;
	RTC_last = 0;
	
	//Set up USART 1
	static const gpio_map_t umap = {
		{AVR32_USART1_RXD_0_0_PIN, AVR32_USART1_RXD_0_0_FUNCTION},
		{AVR32_USART1_TXD_0_0_PIN, AVR32_USART1_TXD_0_0_FUNCTION}
	};
	/*static const gpio_map_t umap = {
		{AVR32_USART3_RXD_0_2_PIN, AVR32_USART3_RXD_0_2_FUNCTION},
		{AVR32_USART3_TXD_0_3_PIN, AVR32_USART3_TXD_0_3_FUNCTION}				
	};*/
	gpio_enable_module(umap, (sizeof(umap) / sizeof(umap[0])));
	
	usart_serial_options_t upot =  {
		.baudrate = 57600,
		.channelmode = USART_NORMAL_CHMODE,
		.charlength = 8,
		.paritytype = USART_NO_PARITY,
		.stopbits = 1
	};
	usart_serial_init(WI_USART, &upot);
	
	//register isr with interrupt controller
	Disable_global_interrupt();
	INTC_init_interrupts();
	
	INTC_register_interrupt(&wifi_usart_rx_isr, AVR32_USART1_IRQ, AVR32_INTC_INT0);
	//INTC_register_interrupt(&wifi_usart_rx_isr, AVR32_USART3_IRQ, AVR32_INTC_INT0);
	
	Enable_global_interrupt();
	
	//enable interrupt RX interrupt
	WI_USART->ier = AVR32_USART_IER_RXRDY_MASK;
}//end wireless_init

//***************************************************************************
//SETTING AND GETTING
//***************************************************************************
char * sixteen_to_eights(uint16_t *data, int size){
	char * eights = malloc(sizeof(uint8_t)*(2*size));
	for (int i = 0; i < 2*size; i += 2)
	{
		eights[i]   = (char)(data[i/2]>>8 && 0xFF);
		eights[i+1] = (char)(data[i/2] && 0xFF);
	}
	return eights;
}

uint16_t * eights_to_sixteen(uint8_t *data, int size){
	uint16_t * sixteens = malloc(sizeof(uint8_t)*(size/2));
	for (int i = 0; i < size/2; i++)
	{
		sixteens[i] = (uint16_t)((data[2*i]<<8) + data[2*i + 1]);
	}
	return sixteens;
}

void set_switch(char* command){
	uint16_t * foo;
	uint8_t bar[6];
	for(int i=0;i<6;i++){
		bar[i] = (uint8_t)command[i+1];
	}
	foo = eights_to_sixteen(bar, 6);
	set_kpid(foo);
	free(foo);
}

void get_switch(char * command){
	uint16_t * data_16;
	char * data_ch;
	switch (command[1])
	{
		case GET_KPD:
			data_16 = malloc(sizeof(uint16_t)*3);
			data_16[0] = (uint16_t)(100**get_kpid_data('p'));
			data_16[1] = (uint16_t)(100**get_kpid_data('r'));
			data_16[2] = (uint16_t)(100**get_kpid_data('y'));
			tx_copy2buff(sixteen_to_eights(data_16, 3), 6);
			free(data_16);
			break;
		case GET_IMU:
			data_16 = malloc(sizeof(uint16_t)*3);
			data_16[0] = (100*imu_get_for_wifi('p'));
			data_16[1] = (100*imu_get_for_wifi('t'));
			data_16[2] = (100*imu_get_for_wifi('h'));
			tx_copy2buff(sixteen_to_eights(data_16, 3), 6);
			free(data_16);
			break;
		case GET_PID_OUT:
			data_ch = get_PID_output();
			tx_copy2buff(data_ch, 4);
			break;
		default:
			break;
	}
}

//***************************************************************************
//communication handling
//***************************************************************************
//returns the length of the packet data.
int rx_sortPacketSize(char rxbuff){
	switch (rxbuff)
	{
	case VELOCITY_VECTOR:
		return VELOCITY_VECTOR_SIZE;
	case MANUAL:
		return MANUAL_SIZE;
	case THROTTLE:
		return THROTTLE_SIZE;
	case VELOCITY_ZERO:
		return VELOCITY_ZERO_SIZE;
	case TOGGLE_DIAGNOSTIC:
		return TOGGLE_DIAGNOSTIC_SIZE;
	case EXTENDED_TELEMETRY:
		return EXTENDED_TELEMETRY_SIZE;
	default:
		return 0;
	}
}//end rx_sortPacketSize


//Polls USART status register for incoming bytes, assembles packets
void rx_postman(void){	
	//Packet Loss Watchdog
	if(rtc_get_value(RTC) >= (RTC_last + 1000)) safetyStop(); //kill if longer than 1000ms since last packet rcvd
	
	//If usart read buffer has new byte, get it
	if(rx_buff_count >= 1){
		
		//Packet is complete if all bytes have been received
		//and last byte is termination character
		int pkt_size = rx_sortPacketSize(rx_buff[rx_cur_loc]);
		if(pkt_size==0){
			rx_buff_count--;
			rx_cur_loc++;
			usart_write_char(WI_USART, 'b');
		}
		else if(rx_buff_count >= pkt_size){
			int pkt_end = rx_cur_loc+pkt_size-1;
			if(pkt_end>=RX_BUFF_SIZE) pkt_end-=RX_BUFF_SIZE;
			
			if(rx_buff[pkt_end] == TERM){
				
				for(int i=0;i<pkt_size;i++){
					int loc = i+rx_cur_loc;
					if(loc>=RX_BUFF_SIZE) loc -= RX_BUFF_SIZE;
					tmp_buff[i] = rx_buff[loc];
				}
				
				rx_logistics(tmp_buff);
				RTC_last = rtc_get_value(RTC);
				rx_cur_loc += pkt_size;
				rx_buff_count -= pkt_size;
				usart_write_char(WI_USART, 'a');
			}
			else{
				rx_buff_count--;
				rx_cur_loc++;
				usart_write_char(WI_USART, 'b');
			}
		}
		if(rx_cur_loc >= RX_BUFF_SIZE) rx_cur_loc -= RX_BUFF_SIZE;
	}//if(usart_test_hit)
}//end rx_postman


//Accepts command packet and orders appropriate action based on command byte. 
void rx_logistics(char* command){
	//gpio_toggle_pin(LED2_GPIO);
	
	char bill[12] = "Hello world!";
		
	switch (command[0]){
	case VELOCITY_VECTOR:
		vector(command);
		break;
	case MANUAL:
		manual(command);
		//gpio_toggle_pin(LED2_GPIO);
		break;
	case THROTTLE:
		throttle(command);
		break;
	case TOGGLE_DIAGNOSTIC:
		tx_copy2buff(bill, 12);
		//diagnosticToggle(command);
		//wireless_test();
		break;
	case SETTER:
		set_switch(command);
		break;
	case GETTER:
		get_switch(command);
		break;
	default:
		for (int i = 0; i < 3; i++)
		{
			bill[i] = command[i];
		}
		//tx_copy2buff(bill, 3);
		break;
	}//end switch
}//end rx_logistics

//Send outgoing bytes when the tx register is empty
void tx_postman(void){
	if(tx_buff_count>0){
		if(usart_write_char(WI_USART, tx_buff[tx_cur_loc]) == USART_SUCCESS){
			tx_cur_loc++;
			tx_buff_count--;
		}
	}
	
	if(tx_cur_loc>=TX_BUFF_SIZE){
		tx_cur_loc -=TX_BUFF_SIZE;
	}
}//end tx_postman


//if the TX buffer is available, copy the input contents to the TX buffer and set size
//Returns 1 if successful, 0 if buffer is in use.
uint8_t tx_copy2buff(char * tx_data, int size){ //this is tx_pkt
	if(tx_buff_count < (TX_BUFF_SIZE-size)){
		for(int i=0;i<size;i++){
			tx_buff[tx_end] = tx_data[i];
			tx_buff_count++;
			tx_end++;
			if(tx_end>=TX_BUFF_SIZE) tx_end-=TX_BUFF_SIZE;
		}
		return 1;
	}
	return 0;
}//end tx_copy2buff


//***************************************************************************
//Command handling
//***************************************************************************

//zero the throttle and collective pitch in the event of signal loss
void safetyStop(void){
	//set_motor(0);
}//end safetyStop


//adjust PID inputs.
void vector(char* velocityVector){
	// imu_input->collective = velocityVector[1];
	// imu_input->pitch = velocityVector[2];
	// imu_input->roll = velocityVector[3];
	// imu_input->yaw = velocityVector[4];
}//end vector


//manually adjust cyclic_x, cyclic_y, tail, and collective
void manual(char* manualVector){
	//global_vals->m_level = throttleHold[1];
}//end manual


//adjust the neutral point of servos in software.
void servoTrim(void){

}//end servoTrim


//adjust the velocity zero to compensate for IMU drift
void zeroAdjust(char * zeroVector){

}//end zeroAdjust


//Adjust throttle hold
void throttle(char* throttleHold){
//	global_vals->m_level = throttleHold[1];
}//end throttle


//Turn on/off additional diagnostic information
void diagnosticToggle(char* toggle){
	//gpio_toggle_pin(LED2_GPIO);
	if(diag==0) diag=1;
	else diag=0;
}//end diagnosticToggle


//send information about the system: orientation
void telemetry(void){

}//end telemetry


//Send extended information about the system: servo trims, ...
void diagnostics(void){
/*	pid_state* pitch_pid = get_PID_data('p');
	pid_state* roll_pid  = get_PID_data('r');
	pid_state* yaw_pid	 = get_PID_data('y');
	long RTCnow = rtc_get_value(&AVR32_RTC);
	char data[8];
	data[0] = 'd';
	data[1] = (char)get_servo_dat('a');
	data[2] = (char)get_servo_dat('b');
	data[3] = (char)get_servo_dat('c');
	data[4] = (char)get_servo_dat('r');
	data[5] = (char)(yaw_pid->Kpid[0]*10.0f);
	data[6] = (char)get_servo_dat('m');
	data[7] = TERM;
	tx_copy2buff(data, 8);
*/
}//end diagnostics


//tests the wireless
void wireless_test(void){
	//toggle LED
	//transmit something encouraging
	//gpio_toggle_pin(LED2_GPIO);
	char hello = "hi evan!";
	tx_copy2buff(&hello, sizeof(hello));
}//end wireless_test