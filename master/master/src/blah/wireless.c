/*
 * wireless.c
 *
 * Created: 2/26/2012 5:30:57 PM
 *  Author: Evan
 */ 

/*NOTE: These functions are written for a cooperative environment.
If interrupts are to be used, care must be taken to protect the receive buffer.
The communication functions work based directly off the functions of ReceiveBuffer*/

#include "wireless.h"
#include "ftdi.h"
#include <usart.h>
#include "led.h"
#include "pwm.h"
#include "PID.h"
#include "imu.h"

#define RTC &AVR32_RTC

char ReceiveBuffer[MAX_PKT_SIZE];
char TransmitBuffer[30];
int diag;

int byte_count_rx;
int byte_count_tx;

int tx_pkt_size;

long RTC_last;
long RTC_now;

void wireless_init(void){
	diag = 0;
	byte_count_rx = 0;
	byte_count_tx = 0;
	tx_pkt_size = 0;
	RTC_last = 0;
	RTC_now = 0;
}

// Command Handling
// ***************************************************************************

int sortPacketSizeRX(char header){
	switch (header)
	{
	case VELOCITY_VECTOR:
		return VELOCITY_VECTOR_SIZE;
		break;
	case SERVO_TRIM:
		return SERVO_TRIM_SIZE;
		break;
	case MANUAL:
		return MANUAL_SIZE;
		break;
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
}
//end sortPacketSizeRX

//if the TX buffer is available, copy the input contents to the TX buffer and set size
void tx_pkt(char *tx_data , int size){
	if (tx_pkt_size == 0)
	{
		for (int i = 0; i < 30; i++)
		{
			TransmitBuffer[i] = tx_data[i];
		}
		tx_pkt_size = size;
	}
}

void tx_service(void){
	//if there are still bytes to be sent, and the UART is available
	if ((byte_count_tx < tx_pkt_size) && (usart_tx_empty(&AVR32_USART1)))
	{
		//write the next byte to the UART and move index to next byte.
		usart_write_char(&AVR32_USART1, TransmitBuffer[byte_count_tx]);
		byte_count_tx++;
		
		//if the entire packet has been sent, reset byte_count_tx and tx_pkt_size
		if (byte_count_tx >= (tx_pkt_size - 1))
		{
			byte_count_tx = 0;
			tx_pkt_size = 0;
		}
	}
}

//Polls USART status register for incoming bytes, a assembles packets
void postman(void){
	
	int debug = 0;
	//debugger
	if(debug==1) gpio_toggle_pin(LED1_GPIO);
	//int fook = 0;
	//usart_read_char(WI_USART,&fook);
	//if(fook!=0) gpio_toggle_pin(LED2_GPIO);
	
	//Packet Loss Watchdog
	RTC_now = rtc_get_value(RTC);
	if(RTC_now >= (RTC_last + 1000)) set_motor(0);
	
	//If usart read buffer has new byte
	if(usart_test_hit(WI_USART)){
		//debugger
		if(debug==1) gpio_toggle_pin(LED2_GPIO);
		
		ReceiveBuffer[byte_count_rx] = usart_getchar(WI_USART);
		byte_count_rx++;
		
		//If packet is complete
		if(byte_count_rx >= (sortPacketSizeRX(ReceiveBuffer[0])+1)){
			byte_count_rx = 0;
			//debugger
			if(debug==1) gpio_toggle_pin(LED0_GPIO);
			logistics(ReceiveBuffer);
			RTC_last = rtc_get_value(RTC);
		}
		
	}//if(usart_test_hit)
	
}//end postman

//Accepts command packet and orders appropriate action based on command byte. 
void logistics(char* command){
	gpio_toggle_pin(LED0_GPIO);
	//Evan modified 3/13
	switch (command[0]){
	case VELOCITY_VECTOR:
		set_PID(command);
		PID_en_flag(1);
		break;
	case MANUAL:
		set_PWM_dat(command);
		PID_en_flag(0);
		break;
	case THROTTLE:
		set_motor(command[1]);
		break;
	case TOGGLE_DIAGNOSTIC:
		gpio_toggle_pin(LED1_GPIO);
		//tx_pkt(imu_get_for_wifi(), 30);
		if (USART_SUCCESS != usart_write_char(WI_USART, 'Q')){
			gpio_toggle_pin(LED2_GPIO);
		}			
		//diagnosticToggle(command);
		break;
	case SERVO_TRIM:
		servoTrim();
		break;
	case EXTENDED_TELEMETRY:
		
		break;
	default:
		break;
	}
	if(diag){
		diagnostics();	
	}
	
	command[0] = 0;
}//end logistics

/*adjust PID inputs.
void vector(char* velocityVector){
	imu_input->collective = velocityVector[1];
	imu_input->pitch = velocityVector[2];
	imu_input->roll = velocityVector[3];
	imu_input->yaw = velocityVector[4];
}//end vector
*/

//adjust the neutral point of servos in software.
void servoTrim(void){
	inc_pid_kp();
}//end servoTrim

/*
//manually adjust cyclic_x, cyclic_y, tail, and collective
void manual(char* manualVector){
	current->collective = manualVector[1];
	current->pitch		= manualVector[2];
	current->roll		= manualVector[3];
	current->yaw		= manualVector[4];
	global_vals->pid_en = 0;
	gpio_toggle_pin(LED2_GPIO);
}//end vector

//adjust the velocity zero to compensate for IMU drift
void zeroAdjust(char* zeroVector){
	current->a = zeroVector[1];
	current->b = zeroVector[2];
	current->c = zeroVector[3];
	current->r = zeroVector[4];
	global_vals->pid_en = 0;
}//end zeroAdjust

//Adjust throttle hold
void throttle(char* throttleHold){
	global_vals->m_level = throttleHold[1];
	//LED_Toggle(0x0002);
}//end throttle
*/
//Turn on/off additional diagnostic information
void diagnosticToggle(char* toggle){
	if(diag==0) diag=1;
	else diag=0;
}//end diagnosticToggle

//***************************************************************************
// Telemetry: downstream information
//***************************************************************************
//Send the standard telemetry data: orientation, velocity, (position)
void telemetry(void){
	//?
}//end telemetry

//Send extended information about the system: servo trims, ...
//maybe have these be locally global so they can be sent back efficiently
void diagnostics(void){
	
//	pid_state* pitch_pid = get_PID_data('p');
//	pid_state* roll_pid  = get_PID_data('r');
	pid_state* yaw_pid	 = get_PID_data('y');
	char a_servo = (char)get_servo_dat('a');
	char b_servo = (char)get_servo_dat('b');
	char c_servo = (char)get_servo_dat('c');
	char r_servo = (char)get_servo_dat('r');
	char m_servo = (char)get_servo_dat('m');
	long RTCnow = rtc_get_value(&AVR32_RTC);
	char data[7];
	data[0] = 'd';
	data[1] = a_servo;
	data[2] = b_servo;
	data[3] = c_servo;
	data[4] = r_servo;
	data[5] = (char)(yaw_pid->Kpid[0]*10.0f);
	data[6] = m_servo;
	com_send_dat (&data, 7);
	//if(RTCnow < rtc_get_value(&AVR32_RTC)+2) gpio_toggle_pin(LED1_GPIO);
	
}//end diagnostics

void wireless_test(void){
	char hi[] = "Hello world! This is UC3A3.\n\r";
	com_send_dat(hi, sizeof(hi)/sizeof(hi[0]));
	
	//gpio_set_pin_low(LED0_GPIO);
	//LED_Off(0x000F); //Turn off all the 
	//LED_On(1);
	//LED_On(2);
}//end 

