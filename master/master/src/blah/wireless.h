/*
 * wireless.h
 *
 * Created: 2/28/2012 6:50:06 PM
 *  Author: Evan
 */ 


#ifndef WIRELESS_H_
#define WIRELESS_H_
#define WI_USART (&AVR32_USART1)

// ***** RX Packet Size and Header ***** \\

#define MAX_PKT_SIZE 			6
#define VELOCITY_VECTOR_SIZE 	4
#define SERVO_TRIM_SIZE 		4
#define MANUAL_SIZE 			4
#define THROTTLE_SIZE 			1
#define VELOCITY_ZERO_SIZE 		4
#define TOGGLE_DIAGNOSTIC_SIZE 	1
#define STANDARD_TELEMETRY_SIZE 1
#define EXTENDED_TELEMETRY_SIZE 1

#define VELOCITY_VECTOR 	'v'
#define SERVO_TRIM 			's'
#define MANUAL 				'm'
#define THROTTLE 			't'
#define STANDARD_TELEMETRY 	'l'
#define EXTENDED_TELEMETRY 	'e'
#define VELOCITY_ZERO 		'z'
#define TOGGLE_DIAGNOSTIC 	'd'

#include "led.h"
#include "asf.h"
#include "PID.h"


//Command Handling
//***************************************************************************
//returns the length of the packet data.
int sortPacketSize(char);

//Polls USART status register for incoming bytes, assembles packets
void postman(void);

//Send outgoing bytes when the tx register is empty
void tx_service(void);

//Accepts command packet and orders appropriate action based on command byte. 
void logistics(char* command);

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

//***************************************************************************
// Telemetry: downstream information
//***************************************************************************
//Send the standard telemetry data: orientation, velocity, (position)
void telemetry(void);

//Send extended information about the system: servo trims, ...
void diagnostics(void);

//tests the wireless
void wireless_test(void);

//sets the memory locations of needed structs
void wireless_init(void);

#endif /* WIRELESS_H_ */