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
	
	rtc_init(&AVR32_RTC, RTC_OSC_32KHZ, 3);
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
	
	/*int two_splats = 0;
	
	while(two_splats < 2)
	{
		if (usart_test_hit(WI_USART))
		{
			if ((WI_USART->rhr & 0xFF) == '*')
			{
				two_splats++;
			}
		}
	}*/
	
	RTC_last_IMU = rtc_get_value(&AVR32_RTC);
	RTC_last_PWM = rtc_get_value(&AVR32_RTC);
	RTC_last_PID = rtc_get_value(&AVR32_RTC);
	
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
		if(RTCnow >= (RTC_last_PID + 5)){
			RTC_last_PID = RTCnow;
			//PID_sequence();
		}
		
		RTCnow = rtc_get_value(&AVR32_RTC);
		//every 20ms update
		if(RTCnow >= (RTC_last_PWM + 40)){
			RTC_last_PWM = RTCnow;
			PWM_update();
		}

		//PWM_tester(&current,'e',10); 
		//pid_sequence();
		//if(current.yaw==current.r)LED_Toggle(LED2);
		//PWM_update();
		
		//PWM_tester('e',100);
		/*
		if(global_vals.pid_en==1){
			pid_sequence();
		}
		*/

		//delay needed to allow motor to go through startup checks
		//delay_ms(5000);
		//set_motor(112);

	}		
}