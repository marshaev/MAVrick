
/* imu.c
 *
 * Created: 1/20/2012 2:48:29 PM
 *  Author: jack
 */ 

#include <asf.h>
#include <stdio.h>
#include <math.h>

//#include <ftdi.h>
#include <MadgwickAHRS.h>
#include <sensors.h>
#include "imu.h"

//Vars
uint16_t ctr = 0;//print counter
int wrap = 0;

float psi_old = 0;

//veloc
float vxyz[3] = {0,0,0};
float phi, theta, psi;

float rtd[3];

void init_imu(void)
{
	init_sensors(&AVR32_SPI1);
	delay_ms(100);
	zero_gyro();
	phi = 0;
	theta = 0;
	psi = 0;
	//zero_accl();
	//kf_init(&kf);
	//(1.0/400);
}

/*
void reset_imu(void)
{
	//kf_init(&kf);
}*/

float atan2f(float y, float x)
{
	if (x > 0)
	{
		return atanf(y/x);
	} 
	if ((y >= 0) && (x < 0))
	{
		return atanf(y/x) + 3.14159265f;
	}
	if ((y < 0) && (x < 0))
	{
		return atanf(y/x) - 3.14159265f;
	}
	if ((y >= 0) && (x == 0))
	{
		return 3.14159265/2;
	}
	if ((y < 0) && (x == 0))
	{
		return -3.14159265/2;
	}
	else
	{
		return 0;
	}
}

/*
float * rotate_world2mav(volatile float *xyz)
{
	float A = q0*q0 - (q1*q1 + q2*q2 + q3*q3);
	
	float B = 2*(q1*xyz[0] + q2*xyz[1] + q3*xyz[2]);
	
	rtd[0] = A*xyz[0] + B*q1 + 2*q0*(q2*xyz[2] - q3*xyz[1]);
	rtd[1] = A*xyz[1] + B*q2 + 2*q0*(q3*xyz[0] - q1*xyz[2]);
	rtd[2] = A*xyz[2] + B*q3 + 2*q0*(q1*xyz[1] - q2*xyz[0]);
	
	return rtd;
	
	//return xyz;
}

void rotate_mav2world(volatile float *xyz)
{
	float q[4] = {q0, - q1, -q2, -q3};//complex conjugate of mav frame
	
	float A = q[0]*q[0] - (q[1]*q[1] + q[2]*q[2] + q[3]*q[3]);
	
	float B = 2*(q[1]*xyz[0] + q[2]*xyz[1] + q[3]*xyz[2]);
	
	rtd[0] = A*xyz[0] + B*q[1] + 2*q[0]*(q[2]*xyz[2] - q[3]*xyz[1]);
	rtd[1] = A*xyz[1] + B*q[2] + 2*q[0]*(q[3]*xyz[0] - q[1]*xyz[2]);
	rtd[2] = A*xyz[2] + B*q[3] + 2*q[0]*(q[1]*xyz[1] - q[2]*xyz[0]);
	
	return &rtd[0];
	
	//return xyz;
}


void velocity(void)
{
	float tran_xyz[3] = {-axyz[1], axyz[0], axyz[2]};
	float *rotat_accl = rotate_world2mav(&tran_xyz[0]);
	
	vxyz[0] += 9.8*rotat_accl[0]/400;
	vxyz[1] += 9.8*rotat_accl[1]/400;
	vxyz[2] += 9.8*(rotat_accl[2] - 1)/400;
	
	for (int i=0; i<3; i++)
	{
		if (vxyz[i] > 0)
		{
			vxyz[i] -= 0.00001;
		}
		if (vxyz[i] < 0)
		{
			vxyz[i] += 0.0001;
		}
	}
	
	//char printer[33];
	//sprintf(printer, "%f,%f,%f \n\r", vxyz[0], vxyz[1], vxyz[2]);
	//com_send_dat(printer, sizeof(printer)/sizeof(printer[0]));
}*/

void service_imu(void)
{
	//char print_buf[38];
	//float head[3] = {0,0,1};
	
	get_imu_data(&AVR32_SPI1);
	MadgwickAHRSupdateIMU(rxyz[0], rxyz[1], rxyz[2], -axyz[1], axyz[0], axyz[2]);
	
	//velocity();
	
	/*float tran_xyz[3] = {-axyz[1], axyz[0], axyz[2]};
	float *rotat_accl = rotate_world2mav(&tran_xyz[0]);
	kf_update(&kf, rotat_accl);
	kf_predict(&kf);*/
	
	
	//rotate_world2mav(axyz);
	
		
	//Euler's
	
	phi = atan2f(2*(q2*q3+q0*q1),(1-2*(q1*q1+q2*q2)));
	theta = asinf(2*(q0*q2-q3*q1));
	float psiD = atan2f(2*(q1*q2+q0*q3),(1-2*(q2*q2+q3*q3)));
		
	phi = 180*phi/3.14159265;
	theta = 180*theta/3.14159265;
	psiD = 180*psiD/3.14159265;
		
	psiD += 180;
		
	if ((psi_old > 340) && (psiD < 20)) wrap++;
	if ((psi_old < 20) && (psiD > 340)) wrap--;
		
	psi = psiD + wrap*360;
	psi_old = psiD;
	
	//sprintf(print_buf, "%f,%f,%f \n\r", phi, theta, psi);
	//com_send_dat(print_buf, sizeof(print_buf)/sizeof(print_buf[0]));
		
	//KF
	/*sprintf(print_buf, "Vxyz:%f %f %f \n\r", kf.kx.vel, kf.ky.vel, kf.kz.vel);
	//sprintf(print_buf, "Axyz:%f %f %f \n\r", kf.kx.acl, kf.ky.acl, kf.kz.acl);
	com_send_dat(print_buf, sizeof(print_buf)/sizeof(print_buf[0]));*/
		
	//heading
	/*sprintf(print_buf, "xyz %f %f %f \n\r", rtd[0], rtd[1], rtd[2]);
	com_send_dat(print_buf, sizeof(print_buf)/sizeof(print_buf[0]));*/
}

float imu_get_for_pid(char axis)
{
	switch (axis){
		case 'r': return theta;
		case 'p': return phi;
		case 'y': return psi;
		default : return 0;
	}
}

uint16_t imu_get_for_wifi(char val)
{	
	switch (val){
		//phi: yaw, rotation around axis
		case 'p': return (uint16_t)(phi);
		//theta: pitch 
		case 't': return (uint16_t)(theta);
		//psi: roll
		case 'h': return (uint16_t)(psi);
		default : return 0;
	}
}

/*
char imu_get_for_wifi(char axis)
{
	if (axis == 'r') return (char)(theta + 0.5f);
	if (axis == 'p ') return (char)(phi + 0.5f);
	if (axis == 'y') return (char)(psi + 0.5f);
}*/