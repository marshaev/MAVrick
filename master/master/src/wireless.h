/*
 * wireless.h
 *
 * Created: 2/28/2012
 * re-factored 4/24/12
 *  Author: Evan
 */ 


#ifndef WIRELESS_H_
#define WIRELESS_H_
#define WI_USART (&AVR32_USART1)
#define RTC &AVR32_RTC

//Packet size includes (header + data bytes + termination char)
#define SERVO_TRIM_SIZE 		4
#define MANUAL_SIZE 			6
#define THROTTLE_SIZE 			3
#define VELOCITY_ZERO_SIZE 		4
#define TOGGLE_DIAGNOSTIC_SIZE 	3
#define STANDARD_TELEMETRY_SIZE 1
#define EXTENDED_TELEMETRY_SIZE 1
#define SETTER_SIZE				15
#define GETTER_SIZE				3

#define RX_BUFF_SIZE 			50
#define TX_BUFF_SIZE			50
#define RX_MAX_SIZE				6

#define VELOCITY_VECTOR_SIZE 	6
//Packet Headders
#define SETTER					's'
#define GETTER					'g'
#define VELOCITY_VECTOR 		'v'
#define MANUAL 					'm'
#define THROTTLE 				't'
#define VELOCITY_ZERO 			'z'
#define TOGGLE_DIAGNOSTIC		'd'
#define STANDARD_TELEMETRY	 	'l'
#define EXTENDED_TELEMETRY		'e'
#define TERM					'\0' //null terminator

#define SET_KPD					1	//KPD pitch roll yaw

//Getter Headers
#define GET_KPD					128	//KPD pitch roll yaw
#define GET_IMU					129	//IMU pitch roll yaw
#define GET_PID_OUT				130	//PID out pitch roll yaw

#include "led.h"
#include "asf.h"
#include "PID.h"
#include "imu.h"
#include <usart.h>

//sets the memory locations of needed structs
void wireless_init(void);

//***************************************************************************
//communication handling
//***************************************************************************
//returns the length of the packet data.
int rx_sortPacketSize(char rxbuff);

//Polls USART status register for incoming bytes, assembles packets
void rx_postman(void);

//Accepts command packet and orders appropriate action based on command byte. 
void rx_logistics(char* command);

//Send outgoing bytes when the tx register is empty
void tx_postman(void);

uint8_t tx_copy2buff(char * tx_data, int size); //this is tx_pkt

char * sixteen_to_eights(uint16_t *data, int size);

uint16_t * eights_to_sixteen(uint8_t *data, int size);

void set_switch(char* command);

void get_switch(char* command);

//***************************************************************************
//Command handling
//***************************************************************************
////zero the throttle and collective pitch in the event of signal loss
void safetyStop(void);

//adjust PID inputs.
void vector(char* velocityVector);

//manually adjust cyclic_x, cyclic_y, tail, and collective
void manual(char* manualVector);

//adjust the neutral point of servos in software.
void servoTrim(void);

//adjust the velocity zero to compensate for IMU drift
void zeroAdjust(char * zeroVector);

//Adjust throttle hold
void throttle(char* throttleHold);

//Turn on/off additional diagnostic information
void diagnosticToggle(char* toggle);

//send information about the system
void telemetry(void);

//Send extended information about the system: servo trims, ...
void diagnostics(void);

//tests the wireless
void wireless_test(void);

#endif /* WIRELESS_H_ */