/*
 * PID.h
 *
 * Created: 3/8/2012 8:20:33 PM
 *  Author: Jack
 */ 


#ifndef PID_H_
#define PID_H_

#include <asf.h>

#define PID_STEP .0025

typedef struct 
{
	float Kpid[3];  //constants for PID equations
	float int_acum; //integral accumulation of all error
	float prev_err; //derivative change from the previous error
} pid_state;

extern void PID_init(void);

extern void set_user_PID(char* user_dat);

extern void set_kpid(uint16_t* kpid_dat);

extern void inc_pid_kp(void);

extern void PID_sequence(void);

extern float PID_update(volatile pid_state *state, float measured, float setpoint);

extern float* get_kpid_data(char pid);

extern char* get_PID_output(void);

extern void PID_en_flag(int on);

#endif /* PID_H_ */