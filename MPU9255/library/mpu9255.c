
#include "mpu9255.h"
// #define ESP8266_IIC
#include "driver/i2c.h"
//////////////////////////////////////////////////////////////////////////////////	 
//开发板 NodeMcu ESP8266
//MPU9255 驱动代码	   
//创建日期:2019/2/10 
//版本：V1.0
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 
 
#define MPU9255_SDA_GPIO     12
#define MPU9255_SCL_GPIO     13

#define I2C_EXAMPLE_MASTER_SCL_IO           MPU9255_SCL_GPIO                /*!< gpio number for I2C master clock */
#define I2C_EXAMPLE_MASTER_SDA_IO           MPU9255_SDA_GPIO               /*!< gpio number for I2C master data  */
#define I2C_EXAMPLE_MASTER_NUM              I2C_NUM_0        /*!< I2C port number for master dev */
#define I2C_EXAMPLE_MASTER_TX_BUF_DISABLE   0                /*!< I2C master do not need buffer */
#define I2C_EXAMPLE_MASTER_RX_BUF_DISABLE   0                /*!< I2C master do not need buffer */

#define MPU6050_SENSOR_ADDR                 0x68             /*!< slave address for MPU6050 sensor */
#define MPU6050_CMD_START                   0x41             /*!< Command to set measure mode */
#define MPU6050_WHO_AM_I                    0x75             /*!< Command to read WHO_AM_I reg */
#define WRITE_BIT                           I2C_MASTER_WRITE /*!< I2C master write */
#define READ_BIT                            I2C_MASTER_READ  /*!< I2C master read */
#define ACK_CHECK_EN                        0x1              /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS                       0x0              /*!< I2C master will not check ack from slave */
#define ACK_VAL                             0x0              /*!< I2C ack value */
#define NACK_VAL                            0x1              /*!< I2C nack value */
#define LAST_NACK_VAL                       0x2              /*!< I2C last_nack value */


//IIC连续写
//addr:器件地址 
//reg:寄存器地址
//len:写入长度
//buf:数据区
//返回值:0,正常
//    其他,错误代码
uint8_t MPU_Write_Len(uint8_t addr,uint8_t reg,uint8_t len,uint8_t *buf)
{
#ifndef ESP8266_IIC
	uint8_t i; 
  	MPU_IIC_Start(); 
	MPU_IIC_Send_Byte((addr<<1)|0);//发送器件地址+写命令	
	if(MPU_IIC_Wait_Ack())	//等待应答
	{
		MPU_IIC_Stop();		 
		return 1;		
	}
  	MPU_IIC_Send_Byte(reg);	//写寄存器地址
  	MPU_IIC_Wait_Ack();		//等待应答
	for(i=0;i<len;i++)
	{
		MPU_IIC_Send_Byte(buf[i]);	//发送数据
		if(MPU_IIC_Wait_Ack())		//等待ACK
		{
			MPU_IIC_Stop();	 
			return 1;		 
		}		
	}    
    MPU_IIC_Stop();	 
	return 0;	
#else
    int ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, addr << 1 | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg, ACK_CHECK_EN);
    i2c_master_write(cmd, buf, len, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    return ret;
#endif
} 
//IIC连续读
//addr:器件地址
//reg:要读取的寄存器地址
//len:要读取的长度
//buf:读取到的数据存储区
//返回值:0,正常
//    其他,错误代码
uint8_t MPU_Read_Len(uint8_t addr,uint8_t reg,uint8_t len,uint8_t *buf)
{ 
#ifndef ESP8266_IIC
 	MPU_IIC_Start(); 
	MPU_IIC_Send_Byte((addr<<1)|0);//发送器件地址+写命令	
	if(MPU_IIC_Wait_Ack())	//等待应答
	{
		MPU_IIC_Stop();		 
		return 1;		
	}
	MPU_IIC_Send_Byte(reg);	//写寄存器地址
	MPU_IIC_Wait_Ack();		//等待应答
	MPU_IIC_Start();
	MPU_IIC_Send_Byte((addr<<1)|1);//发送器件地址+读命令	
	MPU_IIC_Wait_Ack();		//等待应答 
	while(len)
	{
		if(len==1)*buf=MPU_IIC_Read_Byte(0);//读数据,发送nACK 
		else *buf=MPU_IIC_Read_Byte(1);		//读数据,发送ACK  
		len--;
		buf++; 
	}    
    MPU_IIC_Stop();	//产生一个停止条件 
	return 0;	
#else
    int ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, addr << 1 | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    if (ret != ESP_OK) {
        return ret;
    }

    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, addr << 1 | READ_BIT, ACK_CHECK_EN);
    i2c_master_read(cmd, buf, len, LAST_NACK_VAL);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    return ret;
#endif
}


//方向转换
unsigned short inv_row_2_scale(const signed char *row)
{
    unsigned short b;

    if (row[0] > 0)
        b = 0;
    else if (row[0] < 0)
        b = 4;
    else if (row[1] > 0)
        b = 1;
    else if (row[1] < 0)
        b = 5;
    else if (row[2] > 0)
        b = 2;
    else if (row[2] < 0)
        b = 6;
    else
        b = 7;      // error
    return b;
}

//陀螺仪方向控制
unsigned short inv_orientation_matrix_to_scalar(
    const signed char *mtx)
{
    unsigned short scalar; 
    /*
       XYZ  010_001_000 Identity Matrix
       XZY  001_010_000
       YXZ  010_000_001
       YZX  000_010_001
       ZXY  001_000_010
       ZYX  000_001_010
     */

    scalar = inv_row_2_scale(mtx);
    scalar |= inv_row_2_scale(mtx + 3) << 3;
    scalar |= inv_row_2_scale(mtx + 6) << 6;


    return scalar;
}


//MPU9255自测试
//返回值:0,正常
//    其他,失败
uint8_t run_self_test(void)
{
	int result;
	//char test_packet[4] = {0};
	long gyro[3], accel[3]; 
	//result = mpu_run_self_test(gyro, accel);
	result = mpu_run_6500_self_test(gyro, accel, 1);

	
	if (result == 0x7) 
		{
		/* Test passed. We can trust the gyro data here, so let's push it down
		* to the DMP.
		*/


		/*
		 * This portion of the code uses the HW offset registers that are in the MPUxxxx devices
		 * instead of pushing the cal data to the MPL software library
		 */
		unsigned char i = 0;

		for(i = 0; i<3; i++) {
			gyro[i] = (long)(gyro[i] * 32.8f); //convert to +-1000dps
			accel[i] *= 2048.f; //convert to +-16G
			accel[i] = accel[i] >> 16;
			gyro[i] = (long)(gyro[i] >> 16);
		}

		mpu_set_gyro_bias_reg(gyro);
			
		mpu_set_accel_bias_6500_reg(accel);

		return 0;
	}else{return 1;}

// //另一种自检校正方法
// /* Test passed. We can trust the gyro data here, so now we need to update calibrated data*/
// 	long gyro[3], accel[3]; 
// 		unsigned short accel_sens;
//    		float gyro_sens;

// 		mpu_get_accel_sens(&accel_sens);
// 		accel[0] *= accel_sens;
// 		accel[1] *= accel_sens;
// 		accel[2] *= accel_sens;
// 		dmp_set_gyro_bias(gyro);
// 		//inv_set_accel_bias(accel, 3);
// 		mpu_get_gyro_sens(&gyro_sens);
// 		gyro[0] = (long) (gyro[0] * gyro_sens);
// 		gyro[1] = (long) (gyro[1] * gyro_sens);
// 		gyro[2] = (long) (gyro[2] * gyro_sens);
// 		//inv_set_gyro_bias(gyro, 3);
// 				dmp_set_accel_bias(accel);
// 		return 0;
}

/**
 * @brief i2c master initialization
 */
static esp_err_t i2c_example_master_init()
{
    int i2c_master_port = I2C_EXAMPLE_MASTER_NUM;
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_EXAMPLE_MASTER_SDA_IO;
    conf.sda_pullup_en = 0;
    conf.scl_io_num = I2C_EXAMPLE_MASTER_SCL_IO;
    conf.scl_pullup_en = 0;
    ESP_ERROR_CHECK(i2c_driver_install(i2c_master_port, conf.mode));
    ESP_ERROR_CHECK(i2c_param_config(i2c_master_port, &conf));
    return ESP_OK;
}

uint8_t mpu_dmp_init(void)
{
	uint8_t res=0;
	struct int_param_s *int_param=NULL;
#ifndef ESP8266_IIC
	MPU_IIC_Init(); 	//初始化IIC总线
#else
	i2c_driver_delete(I2C_EXAMPLE_MASTER_NUM);
	i2c_example_master_init();
	printf("\n i2c_example_master_init ok \n");
#endif
	
	if(mpu_init(int_param)==0)	//初始化MPU9255
	{	
		printf("mpu init ok\n"); 
		res=mpu_set_sensors(INV_XYZ_GYRO|INV_XYZ_ACCEL|INV_XYZ_COMPASS);//设置所需要的传感器
		if(res) return 1;
		res=mpu_configure_fifo(INV_XYZ_GYRO|INV_XYZ_ACCEL);//设置FIFO
		if(res) return 2; 
		res=mpu_set_sample_rate(DEFAULT_MPU_HZ);	//设置采样率
		if(res) return 3; 
		res=dmp_load_motion_driver_firmware();		//加载dmp固件
		if(res) return 4; 
		res=dmp_set_orientation(inv_orientation_matrix_to_scalar(gyro_orientation));//设置陀螺仪方向
		if(res) return 5; 
		res=dmp_enable_feature(
				DMP_FEATURE_6X_LP_QUAT|				//设置dmp功能
				DMP_FEATURE_TAP| 					//敲击检测 没有实现函数 可屏蔽
		    	DMP_FEATURE_ANDROID_ORIENT| 		//谷歌运动支持 没有实现函数 可屏蔽
				DMP_FEATURE_SEND_RAW_ACCEL|DMP_FEATURE_SEND_CAL_GYRO|
		    	DMP_FEATURE_GYRO_CAL
		);
		if(res) return 6; 
		res=dmp_set_fifo_rate(DEFAULT_MPU_HZ);		//设置DMP输出速率(最大不超过200Hz)
		if(res) return 7;   
		res=run_self_test();		//自检
		if(res) return 8;    
		res=mpu_set_dmp_state(1);	//使能DMP
		if(res) return 9;     
	}else  return 10;
	return 0;
}

//q30格式,long转float时的除数.
#define q30  1073741824.0f

//得到dmp处理后的数据(注意,本函数需要比较多堆栈,局部变量有点多)
//pitch:俯仰角 精度:0.1°   范围:-90.0° <---> +90.0°
//roll:横滚角  精度:0.1°   范围:-180.0°<---> +180.0°
//yaw:航向角   精度:0.1°   范围:-180.0°<---> +180.0°
//返回值:0,正常
//    其他,失败
uint8_t mpu_dmp_get_data(float *pitch,float *roll,float *yaw)
{
	float q0=1.0f,q1=0.0f,q2=0.0f,q3=0.0f;
	unsigned long sensor_timestamp;
	short gyro[3], accel[3], sensors;
	unsigned char more;
	long quat[4]; 
	if(dmp_read_fifo(gyro, accel, quat, &sensor_timestamp, &sensors,&more))return 1;	 
	/* Gyro and accel data are written to the FIFO by the DMP in chip frame and hardware units.
	 * This behavior is convenient because it keeps the gyro and accel outputs of dmp_read_fifo and mpu_read_fifo consistent.
	**/
	/*if (sensors & INV_XYZ_GYRO )
	send_packet(PACKET_TYPE_GYRO, gyro);
	if (sensors & INV_XYZ_ACCEL)
	send_packet(PACKET_TYPE_ACCEL, accel); */
	/* Unlike gyro and accel, quaternions are written to the FIFO in the body frame, q30.
	 * The orientation is set by the scalar passed to dmp_set_orientation during initialization. 
	**/
	if(sensors&INV_WXYZ_QUAT) 
	{
		q0 = quat[0] / q30;	//q30格式转换为浮点数
		q1 = quat[1] / q30;
		q2 = quat[2] / q30;
		q3 = quat[3] / q30; 
		//计算得到俯仰角/横滚角/航向角
		*pitch = asin(-2 * q1 * q3 + 2 * q0* q2)* 57.3;	// pitch
		*roll  = atan2(2 * q2 * q3 + 2 * q0 * q1, -2 * q1 * q1 - 2 * q2* q2 + 1)* 57.3;	// roll
		*yaw   = atan2(2*(q1*q2 + q0*q3),q0*q0+q1*q1-q2*q2-q3*q3) * 57.3;	//yaw
	}else return 2;
	return 0;
}




