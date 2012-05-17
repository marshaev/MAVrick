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
#include <math.h>

//General Vars
struct spi_device gyro_spi_dev, accl_spi_dev;
int16_t iaxyz[3], irxyz[3];
volatile float rxyz[3], axyz[3];
//end General Vars

//ZEROING VARS
# define SAMPLES 100
//rtc
long RTCn = 0;
long RTClst = 0;
//zero data accl
float raw[6][3];
int got_dat[6], got_all;
//zero data gyro
float zerog[3] = {0,0,0};
//output
float a[3], b[3];
int Acal = 0;

//end ZEROING VARS

void init_imu_accl(volatile avr32_spi_t *spi){
	//Register configuration for Accl
	uint8_t wb[] = {
		0b01101100,		//write, multibyte
		0b00001100,		//0x2C datarate; 1010 100hz, 1011 200hz, 1100 400hz
		0b00001000,		//0x2D set to measure
		0b10000000,		//0x2E drdy int enable
		0b01111111,		//0x2F drdy to int 1 else to int 2 pin
		0,				//0x30 NC (int src)
		0b00001010			//0x31 data format
	};
	
	//Write config to accl
	spi_select_device(spi, &accl_spi_dev);
	spi_write_packet(spi, wb, 7);
	spi_deselect_device(spi, &accl_spi_dev);
}

void init_imu_gyro(volatile avr32_spi_t *spi){
		//Register configuration for Gyro
	uint8_t wb[] = {
		0b01100000,		//readNOTwrite, multibyte, 0x20 register
		0b10111111,		//0x20 datarate; 0001 100hz, 0111 200hz, 1011 400hz
		0b00000000,		//0x21 nc
		0b00001000,		//0x22 drdy interrupt
		0b00000000,		//0x23 no change for 250dps range
	};
	
	//Write config to gyro
	spi_select_device(spi, &gyro_spi_dev);
	spi_write_packet(spi, wb, 5);
	spi_deselect_device(spi, &gyro_spi_dev);
}

void init_imu_spi(volatile avr32_spi_t *spi){
//SPI Setup
	spi_master_init(spi);
//Gyro Device Setup
	gyro_spi_dev.id = 0; //CS3
	spi_flags_t gyro_flags = 3;  //Clock Pola/Phas
	spi_master_setup_device(spi, &gyro_spi_dev, gyro_flags, 5000000, gyro_spi_dev.id);
//Accel Device Setup
	accl_spi_dev.id = 1; //CS1
	spi_flags_t accl_flags = 3;  //Clock Pola/Phas
	spi_master_setup_device(spi, &accl_spi_dev, accl_flags, 5000000, accl_spi_dev.id);
//ENABLE SPI
	spi_enable(spi);
//Config GPIO
	static const gpio_map_t SPI_GPIO_MAP =
	{
		{AVR32_SPI1_MISO_0_0_PIN ,AVR32_SPI0_MISO_0_0_FUNCTION},
		{AVR32_SPI1_MOSI_0_0_PIN, AVR32_SPI0_MOSI_0_0_FUNCTION},
		{AVR32_SPI1_SCK_0_0_PIN, AVR32_SPI0_SCK_0_0_FUNCTION},
		{AVR32_SPI1_NPCS_0_0_PIN, AVR32_SPI1_NPCS_0_0_FUNCTION},
		{AVR32_SPI1_NPCS_1_0_PIN, AVR32_SPI1_NPCS_1_0_FUNCTION},
	};
	gpio_enable_module(SPI_GPIO_MAP, sizeof(SPI_GPIO_MAP) / sizeof(SPI_GPIO_MAP[0]));
}

void init_sensors(void)
{
	init_imu_spi(&AVR32_SPI1);
	init_imu_accl(&AVR32_SPI1);
	init_imu_gyro(&AVR32_SPI1);
}

void get_dat_accl (volatile avr32_spi_t *spi)
{
	uint8_t *rb;//readbuff
	uint8_t wb = 0b11110010;//0x32, ReadnotWrite, Multibyte
	spi_select_device(spi, &accl_spi_dev);//chip select low
	spi_write_packet(spi, &wb, 1);
	spi_read_packet(spi, rb, 6);
	spi_deselect_device(spi, &accl_spi_dev);//chip select high
	
	if (Acal)
	{
		iaxyz[0] = rb[0] | (rb[1]<<8);
		axyz[0] = a[0]*(iaxyz[0]/256.0) - b[0];
		iaxyz[1] = rb[2] | (rb[3]<<8);
		axyz[1] = a[1]*(iaxyz[1]/256.0) - b[1];
		iaxyz[2] = rb[4] | (rb[5]<<8);
		axyz[2] = a[2]*(iaxyz[2]/256.0) - b[2];
		
		/*char xyz[40];
		sprintf(xyz, "%f - %f - %f -", axyz[0], axyz[1], axyz[2]);
		com_send_dat(xyz, sizeof(xyz)/sizeof(xyz[0]));*/
	}
	else
	{
		iaxyz[0] = rb[0] | (rb[1]<<8);
		axyz[0] = iaxyz[0]/256.0;
		iaxyz[1] = rb[2] | (rb[3]<<8);
		axyz[1] = iaxyz[1]/256.0;
		iaxyz[2] = rb[4] | (rb[5]<<8);
		axyz[2] = iaxyz[2]/256.0;
	}	
}

void get_dat_gyro (volatile avr32_spi_t *spi)
{
	uint8_t *rb;//readbuff
	uint8_t wb = 0b11101000;//RnotW, MultByte, 0x28
	spi_select_device(spi, &gyro_spi_dev);//chip select low
	spi_write_packet(spi, &wb, 1);
	spi_read_packet(spi, rb, 6);
	spi_deselect_device(spi, &gyro_spi_dev);//chip select high
	
	irxyz[0] = rb[0] | (rb[1]<<8);
	rxyz[0] = irxyz[0]*0.0001527163 - zerog[0];//in rad/s
	irxyz[1] = rb[2] | (rb[3]<<8);
	rxyz[1] = irxyz[1]*0.0001527163 - zerog[1];//in rad/s
	irxyz[2] = rb[4] | (rb[5]<<8);
	rxyz[2] = irxyz[2]*0.0001527163 - zerog[2];//in rad/s
	
	/*char xyz[40];
	sprintf(xyz, "%f - %f - %f -", rxyz[0], rxyz[1], rxyz[2]);
	com_send_dat(xyz, sizeof(xyz)/sizeof(xyz[0]));*/
}

void get_imu_data(volatile avr32_spi_t *spi)
{
	get_dat_accl(spi);
	get_dat_gyro(spi);
}

void get_zero_dat(int index)
{
	uint8_t samp = 0;
	float dat[3] = {0,0,0};
	
	//gpio_set_pin_low(LED2_GPIO);
	delay_ms(2000);
	
	RTClst = rtc_get_value(&AVR32_RTC);
	//Real time loop (5ms)
	while (samp < SAMPLES)
	{
		//Get current time
		RTCn = rtc_get_value(&AVR32_RTC);
		
		//If current time is 5ms from the last time service things
		if(RTCn >= (RTClst + 5))
		{
			RTClst = RTCn;
			
			get_dat_accl(&AVR32_SPI1);
			
			for (int i=0; i<3; i++)
			{
				dat[i] += axyz[i];
			}
			
			samp++;
		}
	}
	
	for (int i=0; i<3; i++)
	{
		raw[index][i] = dat[i]/SAMPLES;
	}
}

float sqr(float a)
{
	return a*a;
}

void zero_accl(void)
{
	for (int i=0; i < 6; i++)
	{
		for (int j=0; j < 3; j++)
		{
			raw[i][j] = 0;
		}
		got_dat[i] = 0;
	}
	
	while(!got_all)
	{
		delay_ms(100);
		get_dat_accl(&AVR32_SPI1);
		
		if ((!got_dat[0]) && (axyz[2] > 0.9))
		{
			get_zero_dat(0);
			got_dat[0] = 1;
		}
		if ((!got_dat[1]) && (axyz[2] < -0.9))
		{
			get_zero_dat(1);
			got_dat[1] = 1;
		}
		if ((!got_dat[2]) && (axyz[1] > 0.9))
		{
			get_zero_dat(2);
			got_dat[2] = 1;
		}
		if ((!got_dat[3]) && (axyz[1] < -0.9))
		{
			get_zero_dat(3);
			got_dat[3] = 1;
		}
		if ((!got_dat[4]) && (axyz[0] > 0.9))
		{
			get_zero_dat(4);
			got_dat[4] = 1;
		}
		if ((!got_dat[5]) && (axyz[0] < -0.9))
		{
			get_zero_dat(5);
			got_dat[5] = 1;
		}
		//gpio_set_pin_high(LED2_GPIO);
		
		got_all = 1;
		for (int i=0; i<6; i++)
		{
			got_all &= got_dat[i];
		}
	}
	
	/*char printer[40];
	for (int i=0; i<6; i++)
	{
		sprintf(printer, "%f | %f | %f |", raw[i][0], raw[i][1], raw[i][2]);
		com_send_dat(printer, sizeof(printer)/sizeof(printer[0]));
	}*/
	
	a[0] = 1; a[1] = 1; a[2] = 1;
	
	/*a[0] = sqrtf(sqr(raw[0][0]-raw[1][0])+sqr(raw[4][0]-raw[5][0]))/2;
	a[1] = sqrtf(sqr(raw[0][1]-raw[1][1])+sqr(raw[2][1]-raw[3][1]))/2;
	a[2] = sqrtf(sqr(raw[4][2]-raw[5][2])+sqr(raw[0][2]-raw[1][2]))/2;*/

	b[0] = (raw[0][0]+raw[1][0]+raw[4][0]+raw[5][0])/4;
	b[1] = (raw[0][1]+raw[1][1]+raw[2][1]+raw[3][1])/4;
	b[2] = (raw[4][2]+raw[5][2]+raw[0][2]+raw[1][2])/4;
	
	delay_ms(500);
	//gpio_set_pin_low(LED2_GPIO);
	delay_ms(4500);
	//gpio_set_pin_high(LED2_GPIO);
	
	Acal = 1;
}

void zero_gyro(void)
{
	uint8_t samp = 0;
	float zerog[3] = {0,0,0};
	
	//gpio_set_pin_low(LED2_GPIO);
	delay_ms(2000);
	
	RTClst = rtc_get_value(&AVR32_RTC);
	//Real time loop (5ms)
	while (samp < SAMPLES)
	{
		//Get current time
		RTCn = rtc_get_value(&AVR32_RTC);
		
		//If current time is 5ms from the last time service things
		if(RTCn >= (RTClst + 5))
		{
			RTClst = RTCn;
			
			get_dat_gyro(&AVR32_SPI1);
			
			for (int i=0; i<3; i++)
			{
				zerog[i] += axyz[i];
			}
			
			samp++;
		}
	}
	
	for (int i=0; i<3; i++)
	{
		zerog[i] = zerog[i]/SAMPLES;
	}
	//gpio_set_pin_high(LED2_GPIO);
}