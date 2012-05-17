#ifndef _PWM_H_
#define _PWM_H_
#include <asf.h>

/*
 _____   _____
|     | |     |
|  a  | |  b  |
|_____| |_____|
     _____
	|     |
	|  c  |
	|_____|
	   |
	   |
	 __|__
	|     |
	|  r  |
	|_____|
	
	
Ranges of inputs:
	Collective:	0->80	(0=off, 80=full throttle [may be altered later])
	Pitch x:	-10->10 & 0<Collective+(Pitch_x*2)<100
	Pitch y:	-20->20 & 0<Collective+Pitch_y<100
	Roll:		-35->35
*/

#define A_BASE	7125
#define	B_BASE	6850
#define C_BASE	7125
#define R_BASE  6550
#define M_BASE	7125
#define R_MAX	175
#define O_MAX	300
#define RC_VAL  7500


extern void PWM_timer_init(void);
extern void PWM_update(void);

/*! \brief Tests the set range of the servos.
 *
 * \param current struct containing current values of the servos.
 * \param servo Char indicating which servos to adjust. values are:
		a front left servo
		b front right servo
		c front rear (bar) servo
		r rear servo
		e all servos
 * \param delay Time(ms) between each step.  Good values would be 50-100.
 */
extern void PWM_tester(char servo, int delay);

extern void set_motor(char setting);

extern int get_servo_dat(char servo);

extern void set_PWM_dat(char* PWM_dat);

#endif 