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
char tmp_buff[RX_BUFF_SIZE]; //intermediate buffer between rx_postman and logistics
uint16_t data_16[6];
char eights[20];
uint16_t sixteens[10];

int diag;			//flag for extended telemetry stuff
int rx_cur_loc;
int rx_end;
int rx_buff_count;	//Number of received bytes in incomplete packet
int tx_cur_loc;
int tx_end;
uint16_t tx_buff_count;
long RTC_last;		//Real-time clock 
int usart_hit;

//rx interrupt service routine
__attribute__((__interrupt__)) static void wifi_usart_rx_isr(void)
{
	rx_buff[rx_end] = WI_USART->rhr & 0xFF;
	
	//echo!!!!
	//usart_write_char(WI_USART, rx_buff[rx_end]);
	
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
	Enable_global_interrupt();
	
	//enable interrupt RX interrupt
	WI_USART->ier = AVR32_USART_IER_RXRDY_MASK;
}//end wireless_init

//***************************************************************************
//SETTING AND GETTING
//***************************************************************************
/*
char * sixteen_to_eights(uint16_t *data, int size){
	char * eights = malloc(sizeof(uint8_t)*(2*size));
	for (int i = 0; i < 2*size; i += 2)
	{
		eights[i]   = (char)(data[i/2]>>8 && 0xFF);
		eights[i+1] = (char)(data[i/2] && 0xFF);
	}
	return eights;
}*/

void eights_to_sixteen(uint8_t *data, int size){
	//uint16_t * sixteens;// = malloc(sizeof(uint16_t)*(size/2));
	for (int i = 0; i < size/2; i++)
	{
		sixteens[i] = ((uint16_t)(data[2*i]))<<8 | (uint16_t)(data[2*i + 1]);
	}
	//return sixteens;
}

//an old set switch that does not switch
/*
void set_switch(char* command){
	uint16_t * ret_16;
	uint8_t tmp_8[12];
	for(int i=0;i<12;i++){
		tmp_8[i] = (uint8_t)command[i+2];
	}
	ret_16 = eights_to_sixteen(tmp_8, 12);
	set_kpid(ret_16);
	free(ret_16);
}*/


void set_switch(char* command){
	uint16_t * ret_16;
	uint8_t tmp_8[20];
	
	switch (command[1])
	{
		case SET_KPD:

			for(int i=0;i<20;i++){
				tmp_8[i] = (uint8_t)command[i+2];
			}
			eights_to_sixteen(tmp_8, 20);
			set_kpid(sixteens);
			beta = ((float)(sixteens[10]))/100;
			break;
		case SET_TRIM:
			set_trim(command);
			break;
		default: break;
	}
	return;
}

void get_switch(char * command){
	volatile uint16_t foo;
	char * data_ch;
	volatile char * ret_16;	
	float * loc_pid;
	switch (command[1])
	{
		case GET_KPD:
			loc_pid = get_kpid_data('p');
			data_16[0] = (uint16_t)(loc_pid[0]*100);
			//data_16[1] = (uint16_t)(loc_pid->Kpid[1]*100);
			data_16[1] = (uint16_t)(loc_pid[2]*100);
			loc_pid = get_kpid_data('r');
			data_16[2] = (uint16_t)(loc_pid[0]*100);
			//data_16[4] = (uint16_t)(loc_pid->Kpid[1]*100);
			data_16[3] = (uint16_t)(loc_pid[2]*100);
			loc_pid = get_kpid_data('y');
			data_16[4] = (uint16_t)(loc_pid[0]*100);
			//data_16[7] = (uint16_t)(loc_pid->Kpid[1]*100);
			data_16[5] = (uint16_t)(loc_pid[2]*100);
			
			for (int i = 0; i < 6; i++)
			{
				eights[2*i]   = (char)(data_16[i]>>8 & 0x00FF);//(data_16[i]>>8 && 0xFF);
				eights[2*i+1] = (char)(data_16[i] & 0x00FF);
			}
			tx_copy2buff(GETTER, GET_KPD, eights, 12);
			break;
		case GET_IMU:
			data_16[0] = (int16_t)(100*imu_get_for_wifi('p'));
			data_16[1] = (int16_t)(100*imu_get_for_wifi('r'));
			data_16[2] = (int16_t)(100*imu_get_for_wifi('y'));
			for (int i = 0; i < 3; i++)
			{
				eights[2*i]   = (char)(data_16[i]>>8 & 0x00FF);//(data_16[i]>>8 && 0xFF);
				eights[2*i+1] = (char)(data_16[i] & 0x00FF);
			}
			tx_copy2buff(GETTER, GET_IMU, eights, 6);
			break;
		case GET_PID_OUT:
			data_ch = get_PID_output();
			tx_copy2buff(GETTER, GET_PID_OUT, data_ch, 4);
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
	case SETTER:
		return SETTER_SIZE;
	case GETTER:
		return GETTER_SIZE;
	case TRIM:
		return TRIM_SIZE;
	default:
		return 0;
	}
}//end rx_sortPacketSize


//Polls USART status register for incoming bytes, assembles packets
void rx_postman(void){	
	//Packet Loss Watchdog
	if(rtc_get_value(RTC) >= (RTC_last + 8000)) safetyStop(); //kill if longer than 1000ms since last packet rcvd
	
	//If usart read buffer has new byte, get it
	if(rx_buff_count >= 1){
		
		//Packet is complete if all bytes have been received
		//and last byte is termination character
		int pkt_size = rx_sortPacketSize(rx_buff[rx_cur_loc]);
		if(pkt_size==0){
			rx_buff_count--;
			rx_cur_loc++;
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
				
				rx_logistics();
				RTC_last = rtc_get_value(RTC);
				rx_cur_loc += pkt_size;
				rx_buff_count -= pkt_size;		
			}
			else{
				rx_buff_count--;
				rx_cur_loc++;
			}
		}
		if(rx_cur_loc >= RX_BUFF_SIZE) rx_cur_loc -= RX_BUFF_SIZE;
	}//if(usart_test_hit)
}//end rx_postman


//Accepts command packet and orders appropriate action based on command byte. 
//void rx_logistics(char* command){
void rx_logistics(void){
	switch (tmp_buff[0]){
	case VELOCITY_VECTOR:
		//set_user_PID(&tmp_buff[0]);
		set_user_PID(tmp_buff[1], tmp_buff[2], tmp_buff[3], tmp_buff[4]);
		break;
	case MANUAL:
		manual(tmp_buff);
		break;
	case THROTTLE:
		throttle(tmp_buff);
		break;
	case TOGGLE_DIAGNOSTIC:
		diagnosticToggle(tmp_buff);
		break;
	case SETTER:
		set_switch(tmp_buff);
		gpio_toggle_pin(LED2_GPIO);
		break;
	case GETTER:
		get_switch(tmp_buff);
		break;
	case TRIM:
		set_coll_trim(tmp_buff);
		break;
	default:
		break;
	}//end switch
	return;
}//end rx_logistics

//Send outgoing bytes when the tx register is empty
void tx_postman(void){
	if(tx_buff_count > 0){
		if (usart_write_char(WI_USART, tx_buff[tx_cur_loc]) == USART_SUCCESS)
		{
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
uint8_t tx_copy2buff(char h1, char h2 ,char * tx_data, int size){ //this is tx_pkt
	//find size of headers
	int offs = 0;
	if (h1)
	{
		offs++;
		if (h2) offs ++;
	}
	
	//if headers and packet are fit into tx_buff add them
	if(tx_buff_count < (TX_BUFF_SIZE-(size+offs+1)))
	{
		//loads header(s)
		
		if (h1)
		{
			tx_buff[tx_end] = h1;
			tx_buff_count++;
			tx_end++;
			if(tx_end>=TX_BUFF_SIZE) tx_end-=TX_BUFF_SIZE;
			if (h2)
			{
				tx_buff[tx_end] = h2;
				tx_buff_count++;
				tx_end++;
				if(tx_end>=TX_BUFF_SIZE) tx_end-=TX_BUFF_SIZE;
			}
		}
		
		//loads packet tx_data into tx_buff
		for(int i=0;i<size;i++){
			tx_buff[tx_end] = tx_data[i];
			tx_buff_count++;
			tx_end++;
			if(tx_end>=TX_BUFF_SIZE) tx_end-=TX_BUFF_SIZE;
		}
		
		tx_buff[tx_end] = '\0';
		tx_end++;
		tx_buff_count++;
		if(tx_end>=TX_BUFF_SIZE) tx_end-=TX_BUFF_SIZE;
		
		return 1;
	}
	return 0;
}//end tx_copy2buff


//***************************************************************************
//Command handling
//***************************************************************************

//zero the throttle and collective pitch in the event of signal loss
void safetyStop(void){
	set_motor(0);
	set_user_PID(127, 100, 50 ,50);
}//end safetyStop


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
	set_motor(throttleHold[1]);
}//end throttle


//Turn on/off additional diagnostic information
void diagnosticToggle(char* toggle){
	char bill[40] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLM";
	tx_copy2buff(0, 0, bill, 40);
	if(diag==0) diag=1;
	else diag=0;
}//end diagnosticToggel


//send information about the system: orientation
void telemetry(void){
	data_16[0] = (int16_t)(100*(imu_get_for_wifi('p') + 180));
	data_16[1] = (int16_t)(100*(imu_get_for_wifi('r') + 180));
	for (int i = 0; i < 2; i++)
	{
		eights[2*i]   = (char)(data_16[i]>>8 & 0x00FF);//(data_16[i]>>8 && 0xFF);
		eights[2*i+1] = (char)(data_16[i] & 0x00FF);
	}
	tx_copy2buff(STANDARD_TELEMETRY, 0, eights, 4);
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
	tx_copy2buff(0, 0, data, 8);
*/
}//end diagnostics


//tests the wireless
void wireless_test(void){
	//transmit something encouraging
	//char * hello = "hi evan!";
	//tx_copy2buff(0, 0, &hello, sizeof(hello));
}//end wireless_test