#ifndef __MPU9255_H
#define __MPU9255_H
#include "mpuiic.h"   								
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"
#include <math.h>
/* Starting sampling rate. */
#define DEFAULT_MPU_HZ  (100)
//			static signed char gyro_orientation[9] = { 	1, 0,  0, 
//														0, 1,  0,              //正常设置                             
//                                                 		0, 0,  1 };
			
			static signed char gyro_orientation[9] = { 	0, 1,  0, 
														1, 0,  0,              //颠倒设置                             
                                                		0, 0,  -1 };																				


//
uint8_t MPU_Write_Len(uint8_t addr,uint8_t reg,uint8_t len,uint8_t *buf);//IIC连续写
uint8_t MPU_Read_Len(uint8_t addr,uint8_t reg,uint8_t len,uint8_t *buf); //IIC连续读 

																								 
uint8_t mpu_dmp_init(void);
uint8_t mpu_dmp_get_data(float *pitch,float *roll,float *yaw);

#endif




































