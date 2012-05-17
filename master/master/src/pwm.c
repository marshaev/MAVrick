#include "pwm.h"
#include <asf.h>
#include "wireless.h"

int collective;
int yaw;
int pitch;
int roll;
int motor;
int t_a;
int t_b;
int t_c;
int t_r;

void PWM_timer_init(void)
{
	collective = 0;
	yaw = 0;
	pitch = 0;
	roll = 0;
	t_a = 0;
	t_b = 0;
	t_c = 0;
	t_r = 0;
	
	//Set up the GPIO pins for output of the timers
	gpio_enable_module_pin(AVR32_TC1_A2_0_PIN, AVR32_TC1_A2_0_FUNCTION);	
	gpio_enable_module_pin(AVR32_TC1_B0_0_PIN, AVR32_TC1_B0_0_FUNCTION);
	gpio_enable_module_pin(AVR32_TC1_B1_0_PIN, AVR32_TC1_B1_0_FUNCTION);
	gpio_enable_module_pin(AVR32_TC0_B2_0_PIN, AVR32_TC0_B2_0_FUNCTION);
	gpio_enable_module_pin(AVR32_TC0_B0_0_0_PIN, AVR32_TC0_B0_0_0_FUNCTION);
	
			
	// Timer waveform options
	const tc_waveform_opt_t waveform_options_b0_1 = {
	//! Channel selection.
	.channel  = 0,
	
	//! Software trigger effect on TIOB.
	.bswtrg   = TC_EVT_EFFECT_NOOP,
	//! External event effect on TIOB.
	.beevt    = TC_EVT_EFFECT_NOOP,
	//! RC compare effect on TIOB.
	.bcpc     = TC_EVT_EFFECT_CLEAR,
	//! RB compare effect on TIOB.
	.bcpb     = TC_EVT_EFFECT_SET,
	
	//! Software trigger effect on TIOA.
	.aswtrg   = TC_EVT_EFFECT_NOOP,
	//! External event effect on TIOA.
	.aeevt    = TC_EVT_EFFECT_NOOP,
	//! RC compare effect on TIOA.
	.acpc     = TC_EVT_EFFECT_NOOP,
	//! RA compare effect on TIOA.
	.acpa     = TC_EVT_EFFECT_NOOP,
	
	//! Waveform selection
	.wavsel   = TC_WAVEFORM_SEL_UP_MODE_RC_TRIGGER,
	//! External event trigger enable.
	.enetrg   = false,
	//! External event selection (non-zero for Channel B to work)
	.eevt     = !0,
	//! External event edge selection.
	.eevtedg  = TC_SEL_NO_EDGE,
	//! Counter disable when RC compare.
	.cpcdis   = false,
	//! Counter clock stopped with RC compare.
	.cpcstop  = false,
	
	//! Burst signal selection.
	.burst    = false,
	//! Clock inversion selection.
	.clki     = false,
	//! Internal source clock 5, fPBA/128.
	.tcclks   = TC_CLOCK_SOURCE_TC4,
	};
	
	const tc_waveform_opt_t waveform_options_b0_0 = {
	//! Channel selection.
	.channel  = 0,
	
	//! Software trigger effect on TIOB.
	.bswtrg   = TC_EVT_EFFECT_NOOP,
	//! External event effect on TIOB.
	.beevt    = TC_EVT_EFFECT_NOOP,
	//! RC compare effect on TIOB.
	.bcpc     = TC_EVT_EFFECT_CLEAR,
	//! RB compare effect on TIOB.
	.bcpb     = TC_EVT_EFFECT_SET,
	
	//! Software trigger effect on TIOA.
	.aswtrg   = TC_EVT_EFFECT_NOOP,
	//! External event effect on TIOA.
	.aeevt    = TC_EVT_EFFECT_NOOP,
	//! RC compare effect on TIOA.
	.acpc     = TC_EVT_EFFECT_NOOP,
	//! RA compare effect on TIOA.
	.acpa     = TC_EVT_EFFECT_NOOP,
	
	//! Waveform selection
	.wavsel   = TC_WAVEFORM_SEL_UP_MODE_RC_TRIGGER,
	//! External event trigger enable.
	.enetrg   = false,
	//! External event selection (non-zero for Channel B to work)
	.eevt     = !0,
	//! External event edge selection.
	.eevtedg  = TC_SEL_NO_EDGE,
	//! Counter disable when RC compare.
	.cpcdis   = false,
	//! Counter clock stopped with RC compare.
	.cpcstop  = false,
	
	//! Burst signal selection.
	.burst    = false,
	//! Clock inversion selection.
	.clki     = false,
	//! Internal source clock 5, fPBA/128.
	.tcclks   = TC_CLOCK_SOURCE_TC4,
	};	
	
	const tc_waveform_opt_t waveform_options_b1 = {
	//! Channel selection.
	.channel  = 1,
	
	//! Software trigger effect on TIOB.
	.bswtrg   = TC_EVT_EFFECT_NOOP,
	//! External event effect on TIOB.
	.beevt    = TC_EVT_EFFECT_NOOP,
	//! RC compare effect on TIOB.
	.bcpc     = TC_EVT_EFFECT_CLEAR,
	//! RB compare effect on TIOB.
	.bcpb     = TC_EVT_EFFECT_SET,
	
	//! Software trigger effect on TIOA.
	.aswtrg   = TC_EVT_EFFECT_NOOP,
	//! External event effect on TIOA.
	.aeevt    = TC_EVT_EFFECT_NOOP,
	//! RC compare effect on TIOA.
	.acpc     = TC_EVT_EFFECT_NOOP,
	//! RA compare effect on TIOA.
	.acpa     = TC_EVT_EFFECT_NOOP,
	
	//! Waveform selection
	.wavsel   = TC_WAVEFORM_SEL_UP_MODE_RC_TRIGGER,
	//! External event trigger enable.
	.enetrg   = false,
	//! External event selection (non-zero for Channel B to work)
	.eevt     = !0,
	//! External event edge selection.
	.eevtedg  = TC_SEL_NO_EDGE,
	//! Counter disable when RC compare.
	.cpcdis   = false,
	//! Counter clock stopped with RC compare.
	.cpcstop  = false,
	
	//! Burst signal selection.
	.burst    = false,
	//! Clock inversion selection.
	.clki     = false,
	//! Internal source clock 5, fPBA/128.
	.tcclks   = TC_CLOCK_SOURCE_TC4,
	};	
	
	const tc_waveform_opt_t waveform_options_b2 = {
	//! Channel selection.
	.channel  = 2,
	
	//! Software trigger effect on TIOB.
	.bswtrg   = TC_EVT_EFFECT_NOOP,
	//! External event effect on TIOB.
	.beevt    = TC_EVT_EFFECT_NOOP,
	//! RC compare effect on TIOB.
	.bcpc     = TC_EVT_EFFECT_CLEAR,
	//! RB compare effect on TIOB.
	.bcpb     = TC_EVT_EFFECT_SET,
	
	//! Software trigger effect on TIOA.
	.aswtrg   = TC_EVT_EFFECT_NOOP,
	//! External event effect on TIOA.
	.aeevt    = TC_EVT_EFFECT_NOOP,
	//! RC compare effect on TIOA.
	.acpc     = TC_EVT_EFFECT_CLEAR,
	//! RA compare effect on TIOA.
	.acpa     = TC_EVT_EFFECT_SET,
	
	//! Waveform selection
	.wavsel   = TC_WAVEFORM_SEL_UP_MODE_RC_TRIGGER,
	//! External event trigger enable.
	.enetrg   = false,
	//! External event selection (non-zero for Channel B to work)
	.eevt     = !0,
	//! External event edge selection.
	.eevtedg  = TC_SEL_NO_EDGE,
	//! Counter disable when RC compare.
	.cpcdis   = false,
	//! Counter clock stopped with RC compare.
	.cpcstop  = false,
	
	//! Burst signal selection.
	.burst    = false,
	//! Clock inversion selection.
	.clki     = false,
	//! Internal source clock 5, fPBA/128.
	.tcclks   = TC_CLOCK_SOURCE_TC4,
	};	
	
	const tc_waveform_opt_t waveform_options_a2 = {
	//! Channel selection.
	.channel  = 2,
	
	//! Software trigger effect on TIOB.
	.bswtrg   = TC_EVT_EFFECT_NOOP,
	//! External event effect on TIOB.
	.beevt    = TC_EVT_EFFECT_NOOP,
	//! RC compare effect on TIOB.
	.bcpc     = TC_EVT_EFFECT_NOOP,
	//! RB compare effect on TIOB.
	.bcpb     = TC_EVT_EFFECT_NOOP,
	
	//! Software trigger effect on TIOA.
	.aswtrg   = TC_EVT_EFFECT_NOOP,
	//! External event effect on TIOA.
	.aeevt    = TC_EVT_EFFECT_NOOP,
	//! RC compare effect on TIOA.
	.acpc     = TC_EVT_EFFECT_CLEAR,
	//! RA compare effect on TIOA.
	.acpa     = TC_EVT_EFFECT_SET,
	
	//! Waveform selection
	.wavsel   = TC_WAVEFORM_SEL_UP_MODE_RC_TRIGGER,
	//! External event trigger enable.
	.enetrg   = false,
	//! External event selection (non-zero for Channel B to work)
	.eevt     = !0,
	//! External event edge selection.
	.eevtedg  = TC_SEL_NO_EDGE,
	//! Counter disable when RC compare.
	.cpcdis   = false,
	//! Counter clock stopped with RC compare.
	.cpcstop  = false,
	
	//! Burst signal selection.
	.burst    = false,
	//! Clock inversion selection.
	.clki     = false,
	//! Internal source clock 5, fPBA/128.
	.tcclks   = TC_CLOCK_SOURCE_TC4,
	};	

	// Setup timer/counter waveform mode
	sysclk_enable_peripheral_clock(&AVR32_TC0);
	sysclk_enable_peripheral_clock(&AVR32_TC1);
	tc_init_waveform(&AVR32_TC1, &waveform_options_b1);
	tc_init_waveform(&AVR32_TC1, &waveform_options_b0_1);
	tc_init_waveform(&AVR32_TC1, &waveform_options_a2);
	tc_init_waveform(&AVR32_TC0, &waveform_options_b0_0);
	tc_init_waveform(&AVR32_TC0, &waveform_options_b2);
	
	
	tc_write_rb(&AVR32_TC1, 0, B_BASE);  
	tc_write_rc(&AVR32_TC1, 0, RC_VAL); 
	tc_write_rb(&AVR32_TC1, 1, R_BASE);   
	tc_write_rc(&AVR32_TC1, 1, RC_VAL); 
	tc_write_ra(&AVR32_TC1, 2, A_BASE);
	tc_write_rc(&AVR32_TC1, 2, RC_VAL); 
	
	tc_write_rb(&AVR32_TC0, 0, M_BASE);   
	tc_write_rc(&AVR32_TC0, 0, RC_VAL); 
	tc_write_rb(&AVR32_TC0, 2, C_BASE);   
	tc_write_rc(&AVR32_TC0, 2, RC_VAL); 
	
	// Start the timer PWM channels
	tc_start(&AVR32_TC1, 2);
	tc_start(&AVR32_TC1, 0);
	tc_start(&AVR32_TC1, 1);
	tc_start(&AVR32_TC0, 0);
	tc_start(&AVR32_TC0, 2);	
}

void PWM_tester(char servo, int delay){	
	if(servo=='a'){
		for(int i=0;i<=O_MAX;i++){
			tc_write_ra(&AVR32_TC1, 2, A_BASE-i);
			if(i==0) delay_ms(3000);
			else delay_ms(delay);
		}
	}
	else if(servo=='b'){
		for(int i=0;i<=O_MAX;i++){
			tc_write_rb(&AVR32_TC1, 0, B_BASE+i);
			if(i==0) delay_ms(3000);
			else delay_ms(delay);
		}
	}
	else if(servo=='c'){
		for(int i=0;i<=O_MAX;i++){
			tc_write_rb(&AVR32_TC0, 2, C_BASE-i);
			if(i==0) delay_ms(3000);
			else delay_ms(delay);
		}
	}
	else if(servo=='r'){
		for(int i=0;i<=R_MAX;i++){
			tc_write_rb(&AVR32_TC1, 1, R_BASE+i);
			if(i==0) delay_ms(3000);
			else delay_ms(delay);
		}
	}
	else if(servo=='e'){
		for(int i=0;i<O_MAX;i++){
			tc_write_ra(&AVR32_TC1, 2, A_BASE-i);
			tc_write_rb(&AVR32_TC1, 0, B_BASE+i);
			tc_write_rb(&AVR32_TC0, 2, C_BASE-i);
			if(i<R_MAX)	tc_write_rb(&AVR32_TC1, 1, R_BASE-i);
			
			//tc_write_rb(&AVR32_TC0, 0, 255);
			if(i==0) delay_ms(3000);
			else delay_ms(delay);
		}
	}
}

void PWM_update(void){
	//gpio_toggle_pin(LED1_GPIO);	
	t_a = collective-pitch+roll;
	t_b = collective-pitch-roll;
	t_c = collective+pitch+pitch;
	t_r = yaw;
	
	//if(collective!=0 || yaw !=0 || pitch!=0 || roll !=0) gpio_toggle_pin(LED0_GPIO);

	tc_write_ra(&AVR32_TC1, 2, A_BASE-t_a);
	tc_write_rb(&AVR32_TC1, 0, B_BASE+t_b);
	tc_write_rb(&AVR32_TC0, 2, C_BASE-t_c);
	tc_write_rb(&AVR32_TC1, 1, R_BASE+t_r);
}	

void set_motor(char setting){
	int level = (int) setting;
	if(level>100) level = 100;
	level = (int)(level * 3.75f);
	/*
	switch(setting){
	case 'a':
		level = 0;
		break;
	case 'b':
		level = 25;
		break;
	case 'c':
		level = 50;
		break;
	case 'd':
		level = 75;
		break;
	case 'e':
		level = 100;
		break;
	default:
		level = 0;
		break;
	}
	*/
	tc_write_rb(&AVR32_TC0, 0, M_BASE - level);
	motor = level;
}

int get_servo_dat(char servo){
	switch(servo){
	case 'a':
		return t_a;
		break;
	case 'b':
		return t_b;
		break;
	case 'c':
		return t_c;
		break;
	case 'r':
		return t_r;
		break;
	case 'o':
		return collective;
		break;
	case 'y':
		return yaw;
		break;
	case 'p':
		return pitch;
		break;
	case 'l':
		return roll;
		break;
	case 'm':
		return motor;
		break;
	default:
		return 0;
		break;
	}
}

void set_PWM_dat(char* PWM_dat){
	/*
	[0] = packet type
	[1] = collective
	[2] = yaw
	[3] = pitch
	[4] = roll
	*/
	//gpio_toggle_pin(LED2_GPIO);
	collective	= (int)PWM_dat[1];
	yaw			= (int)(PWM_dat[2]);
	pitch		= (int)PWM_dat[3];
	roll		= (int)PWM_dat[4];
}