/*
 * sensors.h
 *
 * Created: 3/10/2012 7:12:20 PM
 *  Author: Jack
 */ 


#ifndef SENSORS_H_
#define SENSORS_H_

/*
 * sensors.c
 *
 * Created: 3/10/2012 7:12:09 PM
 *  Author: Jack
 */ 

#include <asf.h>
//#include <ftdi.h>
#include <MadgwickAHRS.h>
#include <stdio.h>

extern float rxyz[3], axyz[3];

void init_sensors(volatile avr32_spi_t *spi);

void get_imu_data(volatile avr32_spi_t *spi);

void zero_accl(void);

void zero_gyro(void);

#endif /* SENSORS_H_ */