/**
 * \file
 *
 * \brief Empty user application template
 *
 */

/*
 * Include header files for all drivers that have been imported from
 * AVR Software Framework (ASF).
 */
#include <asf.h>
#include "pwm.h"
#include "PID.h"
#include "wireless.h"
//#include "ftdi.h"
#include <imu.h>

long RTCnow = 0;
long RTC_last_IMU = 0;
long RTC_last_PWM = 0;
long RTC_last_PID = 0;
long RTC_last_TEL = 0;

#define RTC &AVR32_RTC

void init_xplained (void)
{
	//Init Xplained Board
	board_init();
	
	//Init ASF drivers
	sysclk_init();
	delay_init(sysclk_get_main_hz());
	sysclk_enable_pba_module(SYSCLK_SPI0);
	sysclk_enable_pba_module(SYSCLK_USART3);
	sysclk_enable_pba_module(SYSCLK_USART1);
	
	wireless_init();
	
	rtc_init(&AVR32_RTC, RTC_OSC_32KHZ, 1);//8khz
	rtc_enable(&AVR32_RTC);
	PWM_timer_init();
	PID_init();
	
	//initializes Controls software
	init_imu();
}

int main (void)

{
	/*
	zero = 7300;
	1ms  = 7125;
	2ms  = 6750;
	*/
	init_xplained();
	
	RTC_last_IMU = rtc_get_value(&AVR32_RTC);
	RTC_last_PWM = RTC_last_IMU;
	RTC_last_PID = RTC_last_IMU;
	RTC_last_TEL = RTC_last_IMU;

	while(1){
			
		rx_postman();	
		tx_postman();
		
		RTCnow = rtc_get_value(&AVR32_RTC);
		//every 2.5ms update
		if(RTCnow >= (RTC_last_IMU + 5)){
			RTC_last_IMU = RTCnow;
			service_imu();
			//if(RTCnow == rtc_get_value(RTC)+1) gpio_set_pin_low(LED0_GPIO);
		}
		
		RTCnow = rtc_get_value(&AVR32_RTC);
		//every 20ms update
		if(RTCnow >= (RTC_last_PWM + 400)){
			RTC_last_PWM = RTCnow;
			PID_sequence();
			PWM_update();
		}
		
		RTCnow = rtc_get_value(&AVR32_RTC);
		//every 250ms update
		if(RTCnow >= (RTC_last_TEL + 2000)){
			RTC_last_TEL = RTCnow;
			telemetry();
		}
	}	

}