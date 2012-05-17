/*
 * PID.c
 *
 * Created: 3/8/2012 8:20:19 PM
 *  Author: Jack
 */ 

#include <asf.h>
#include <pwm.h>
#include <PID.h>
#include "wireless.h"
#include "imu.h"
#include <math.h>

pid_state* pitch_angle_pid;
pid_state* roll_angle_pid;
pid_state* yaw_angle_pid;

char usr_coll;
float usr_yaw;
float usr_pit;
float usr_roll;

char pid_output[4];
int en;
/*
[0] = collective
[1] = yaw
[2] = pitch
[3] = roll
*/

void PID_init(void)
{
	pitch_angle_pid = malloc(sizeof(pid_state));
	pitch_angle_pid->Kpid[0] = 0;
	pitch_angle_pid->Kpid[1] = 0;
	pitch_angle_pid->Kpid[2] = 0;
	roll_angle_pid  = malloc(sizeof(pid_state));
	roll_angle_pid->Kpid[0] = 0;
	roll_angle_pid->Kpid[1] = 0;
	roll_angle_pid->Kpid[2] = 0;
	yaw_angle_pid   = malloc(sizeof(pid_state));
	yaw_angle_pid->Kpid[0] = 0;
	yaw_angle_pid->Kpid[1] = 0;
	yaw_angle_pid->Kpid[2] = 0;
	en = 1;
}

void set_user_PID(char* user_dat){
	usr_coll = user_dat[1];
	usr_yaw	 = (float)user_dat[2];
	usr_pit	 = (float)user_dat[3];
	usr_roll = (float)user_dat[4];
}

void set_kpid(uint16_t* kpid_dat){
	pitch_angle_pid->Kpid[0] = (float)(kpid_dat[0]/100);
	pitch_angle_pid->Kpid[2] = (float)(kpid_dat[1]/100);
	roll_angle_pid->Kpid[0]  = (float)(kpid_dat[2]/100);
	roll_angle_pid->Kpid[2]  = (float)(kpid_dat[3]/100);
	yaw_angle_pid->Kpid[0]   = (float)(kpid_dat[4]/100);
	yaw_angle_pid->Kpid[2]   = (float)(kpid_dat[5]/100);
}

void inc_pid_kp(void){
	yaw_angle_pid->Kpid[0] += .1;
}

void PID_sequence(void)
{
	if(en){
		float imu_yaw  = imu_get_for_pid('y');
		float imu_pit  = imu_get_for_pid('p');
		float imu_roll = imu_get_for_pid('r');
		
		//gpio_toggle_pin(LED0_GPIO);
		pid_output[0]	= usr_coll;
		pid_output[1]	= (char)(PID_update(yaw_angle_pid, usr_yaw, imu_yaw)+0.5f);
		pid_output[2]	= (char)(PID_update(pitch_angle_pid, usr_pit, imu_pit)+0.5f);
		pid_output[3]	= (char)(PID_update(roll_angle_pid, usr_roll, imu_roll)+0.5f);
		//gpio_toggle_pin(LED1_GPIO);
		set_PWM_dat(pid_output);
	}		
}

float PID_update(volatile pid_state *state, float measured, float setpoint)
{	
	float error = setpoint - measured;
	float avg_err = (error - state->prev_err)/2;
	
	//P
	float output = (state->Kpid[0])*error;
	
	//I
	state->int_acum = PID_STEP*avg_err + state->int_acum;
	output += (state->Kpid[1])*state->int_acum;
	
	//D
	output += state->Kpid[2]*((error-state->prev_err)/PID_STEP);
	
	return output;
}

float* get_kpid_data(char pid){
	float* foo;
	if(pid=='p'){
		foo = pitch_angle_pid->Kpid;
	}
	else if(pid=='r'){
		foo = roll_angle_pid->Kpid;
	}
	else{
		foo = yaw_angle_pid->Kpid;
	}
	return foo;
}

char* get_PID_output(){
	return pid_output;
}

void PID_en_flag(int on){
	en = on;
}
