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
float coll_trim;

int16_t pid_output[4];
int en;
/*
[0] = collective
[1] = yaw
[2] = pitch
[3] = roll
*/

uint8_t colect_trim;

void PID_init(void)
{
	pitch_angle_pid = malloc(sizeof(pid_state));
	pitch_angle_pid->Kpid[0]  = 3;
	pitch_angle_pid->Kpid[1]  = 1.66666666666;
	pitch_angle_pid->Kpid[2]  = 0;
	pitch_angle_pid->int_acum = 0;
	pitch_angle_pid->prev_err = 0;
	pitch_angle_pid->int_prev = 0;
	roll_angle_pid  = malloc(sizeof(pid_state));
	roll_angle_pid->Kpid[0]  = 3;
	roll_angle_pid->Kpid[1]  = 1.66666666666;
	roll_angle_pid->Kpid[2]  = 0;
	roll_angle_pid->int_acum = 0;
	roll_angle_pid->prev_err = 0;
	roll_angle_pid->int_prev = 0;
	yaw_angle_pid   = malloc(sizeof(pid_state));
	yaw_angle_pid->Kpid[0]  = 3;
	yaw_angle_pid->Kpid[1]  = 0.243;
	yaw_angle_pid->Kpid[2]  = 0;
	yaw_angle_pid->int_acum = 0;
	yaw_angle_pid->prev_err = 0;
	yaw_angle_pid->int_prev = 0;
	en = 1;
	usr_coll = 0;
	usr_yaw = 0;
	usr_pit = 0;
	usr_roll = 0;
}

/*
void set_user_PID(char* user_dat){
	usr_coll = (float)(user_dat[1]) + colect_trim;
	usr_yaw	 = (28/20)*((float)user_dat[2]) - 100;
	usr_pit	 = (65/100)*((float)user_dat[3]) - 50;
	usr_roll = (65/100)*((float)user_dat[4]) - 50;
}*/

void set_user_PID(char u_col, char u_yaw, char u_pit, char u_rol){
	usr_coll = (float)u_col;// + colect_trim;
	int tmp1 = (int)u_yaw;
	int tmp2 = (int)u_pit;
	int tmp3 = (int)u_rol;
	usr_yaw	 = ((float)(tmp1 - 100)) * (1.8);
	usr_pit	 = ((float)(tmp2 - 50)) * (.3);
	usr_roll = ((float)(tmp3 - 50)) * (.3);
	//usr_roll = (.65)*(((float)u_rol) - 50);
}

void set_kpid(uint16_t* kpid_dat){
	pitch_angle_pid->Kpid[0] = ((float)(kpid_dat[0]))/100;
	pitch_angle_pid->Kpid[1] = ((float)(kpid_dat[1]))/100;
	pitch_angle_pid->Kpid[2] = ((float)(kpid_dat[2]))/100;
	roll_angle_pid->Kpid[0]  = ((float)(kpid_dat[3]))/100;
	roll_angle_pid->Kpid[1]  = ((float)(kpid_dat[4]))/100;
	roll_angle_pid->Kpid[2]  = ((float)(kpid_dat[5]))/100;
	yaw_angle_pid->Kpid[0]   = ((float)(kpid_dat[6]))/100;
	yaw_angle_pid->Kpid[1]   = ((float)(kpid_dat[7]))/100;
	yaw_angle_pid->Kpid[2]   = ((float)(kpid_dat[8]))/100;
}

void set_trim(char *data){
	colect_trim = data[2];
}

void PID_sequence(void)
{
	if(en){
		float imu_yaw  = imu_get_for_pid('y');
		float imu_pit  = imu_get_for_pid('p');
		float imu_roll = imu_get_for_pid('r');
		
		//gpio_toggle_pin(LED0_GPIO);
		pid_output[0]	= usr_coll - coll_trim;
		pid_output[1]	= -(int16_t)(PID_update(yaw_angle_pid, imu_yaw, -usr_yaw)+0.5f);
		pid_output[2]	= (int16_t)(PID_update(pitch_angle_pid, imu_pit, usr_pit)+0.5f);
		/////////////////////////////////////////////////////////
		//Emergency magic number alert: 6 deg is the angular discrepancy in the mounting :-)
		pid_output[3]	= (int16_t)(PID_update(roll_angle_pid, imu_roll+6, usr_roll)+0.5f);
		/////////////////////////////////////////////////////////
		/*pid_output[1]	= (char)(PID_update(yaw_angle_pid, usr_yaw, imu_yaw)+0.5f);
		pid_output[2]	= (char)(PID_update(pitch_angle_pid, usr_pit, imu_pit)+0.5f);
		pid_output[3]	= (char)(PID_update(roll_angle_pid, usr_roll, imu_roll)+0.5f);*/
		//gpio_toggle_pin(LED1_GPIO);
		set_PWM_dat(pid_output, 'p');
		//pid_output[0] = coll
		//pid_output[1] = yaw
		//pid_output[2] = pitch
		//pid_output[3] = roll
		int tmp_yaw   = get_servo_dat('y');
		int tmp_pitch = get_servo_dat('p');
		int tmp_roll  = get_servo_dat('l');
		if(pid_output[1]!=tmp_yaw)   yaw_angle_pid->int_acum   = yaw_angle_pid->int_prev;
		if(pid_output[2]!=tmp_pitch) pitch_angle_pid->int_acum = pitch_angle_pid->int_prev;
		if(pid_output[3]!=tmp_roll)  roll_angle_pid->int_acum  = roll_angle_pid->int_prev;
	}		
}

float PID_update(volatile pid_state *state, float measured, float setpoint)
{	
	volatile float error = setpoint - measured;
	volatile float avg_err = (error + state->prev_err)/2;
	
	//P
	volatile float output = (state->Kpid[0])*error;
	
	//I
	state->int_prev = state->int_acum;
	state->int_acum = (PID_STEP*avg_err) + state->int_acum;
	output += (state->Kpid[1])*state->int_acum;
	
	//D
	output += state->Kpid[2]*((error - state->prev_err)/PID_STEP);
	state->prev_err = error;
	
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

void set_coll_trim(char * command){
	coll_trim = (float)(command[1]);
}