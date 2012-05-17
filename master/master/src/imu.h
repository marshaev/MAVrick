/*
 * IMU.h
 *
 * Created: 1/20/2012 3:45:30 PM
 *  Author: jack
 */ 


#ifndef IMU_H_
#define IMU_H_

void init_imu(void);

void reset_imu(void);

void service_imu(void);

float imu_get_for_pid(char axis);

uint16_t imu_get_for_wifi(char val);

#include <math.h>

#endif /* IMU_H_ */