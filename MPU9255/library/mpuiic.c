#include "mpuiic.h"
#include "driver/i2c.h"
#include "driver/gpio.h"

#define delay_us os_delay_us

#define MPU9255_SDA_GPIO     12 //(D6)
#define MPU9255_SCL_GPIO     13 //(D7)

#define I2C_MASTER_SDA_GPIO MPU9255_SDA_GPIO
#define I2C_MASTER_SCL_GPIO MPU9255_SCL_GPIO
//#define gpio_set_level(pin,val)  gpio_set_level(pin, val)

 //MPU IIC 延时函数
void MPU_IIC_Delay(void)
{
	delay_us(2);
}

//初始化IIC
void MPU_IIC_Init(void)
{					     
    printf( "init MPU_IIC\n");

    i2c_config_t i2c_conf;

    i2c_conf.mode               = I2C_MODE_MASTER;
    i2c_conf.sda_io_num         = MPU9255_SDA_GPIO;
    i2c_conf.sda_pullup_en      = GPIO_PULLUP_ENABLE;
    i2c_conf.scl_io_num         = MPU9255_SCL_GPIO;
    i2c_conf.scl_pullup_en      = GPIO_PULLUP_ENABLE;

    // i2c_param_config(I2C_NUM_0, &i2c_conf);
    i2c_set_pin(I2C_NUM_0, i2c_conf.sda_io_num, i2c_conf.scl_io_num,
                            i2c_conf.sda_pullup_en, i2c_conf.scl_pullup_en, i2c_conf.mode);

    printf("init MPU9255");
    printf("\nOLED_SDA_GPIO:%d  OLED_SCL_GPIO:%d",i2c_conf.sda_io_num, i2c_conf.scl_io_num);
}

//产生IIC起始信号
void MPU_IIC_Start(void)
{
    gpio_set_direction(I2C_MASTER_SDA_GPIO,GPIO_MODE_OUTPUT);//SDA_OUT();
    gpio_set_level(I2C_MASTER_SDA_GPIO,1);//IIC_SDA=1;
    gpio_set_level(I2C_MASTER_SCL_GPIO,1);//IIC_SCL=1;
    delay_us(2);
    gpio_set_level(I2C_MASTER_SDA_GPIO,0);//IIC_SDA=0;
    delay_us(2);
    gpio_set_level(I2C_MASTER_SCL_GPIO,0);//IIC_SCL=0;
    delay_us(2);
}	  
//产生IIC停止信号
void MPU_IIC_Stop(void)
{
	gpio_set_direction(I2C_MASTER_SDA_GPIO,GPIO_MODE_OUTPUT);//MPU_SDA_OUT();//sda线输出
	gpio_set_level(I2C_MASTER_SCL_GPIO,0);//MPU_IIC_SCL=0;
	gpio_set_level(I2C_MASTER_SDA_GPIO,0);//MPU_IIC_SDA=0;//STOP:when CLK is high DATA change form low to high
 	MPU_IIC_Delay();
	gpio_set_level(I2C_MASTER_SCL_GPIO,1);//MPU_IIC_SCL=1; 
	gpio_set_level(I2C_MASTER_SDA_GPIO,1);//MPU_IIC_SDA=1;//发送I2C总线结束信号
	MPU_IIC_Delay();			
}
//等待应答信号到来
//返回值：1，接收应答失败
//        0，接收应答成功
uint8_t MPU_IIC_Wait_Ack(void)
{
	uint8_t ucErrTime=0;
	gpio_set_direction(I2C_MASTER_SDA_GPIO,GPIO_MODE_INPUT);//MPU_SDA_IN();      //SDA设置为输入  
	gpio_set_level(I2C_MASTER_SDA_GPIO,1);//MPU_IIC_SDA=1;
	MPU_IIC_Delay();	   
	gpio_set_level(I2C_MASTER_SCL_GPIO,1);//MPU_IIC_SCL=1;
	MPU_IIC_Delay();	 
	while(gpio_get_level(I2C_MASTER_SDA_GPIO))
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			MPU_IIC_Stop();
			return 1;
		}
	}
	gpio_set_level(I2C_MASTER_SCL_GPIO,0);//MPU_IIC_SCL=0;//时钟输出0 	   
	return 0;  
} 
//产生ACK应答
void MPU_IIC_Ack(void)
{
	gpio_set_level(I2C_MASTER_SCL_GPIO,0);//MPU_IIC_SCL=0;
	gpio_set_direction(I2C_MASTER_SDA_GPIO,GPIO_MODE_OUTPUT);//MPU_SDA_OUT();
	gpio_set_level(I2C_MASTER_SDA_GPIO,0);//MPU_IIC_SDA=0;
	MPU_IIC_Delay();
	gpio_set_level(I2C_MASTER_SCL_GPIO,1);//MPU_IIC_SCL=1;
	MPU_IIC_Delay();
	gpio_set_level(I2C_MASTER_SCL_GPIO,0);//MPU_IIC_SCL=0;
}
//不产生ACK应答		    
void MPU_IIC_NAck(void)
{
	gpio_set_level(I2C_MASTER_SCL_GPIO,0);//MPU_IIC_SCL=0;
	gpio_set_direction(I2C_MASTER_SDA_GPIO,GPIO_MODE_OUTPUT);//MPU_SDA_OUT();
	gpio_set_level(I2C_MASTER_SDA_GPIO,1);//MPU_IIC_SDA=1;
	MPU_IIC_Delay();
	gpio_set_level(I2C_MASTER_SCL_GPIO,1);//MPU_IIC_SCL=1;
	MPU_IIC_Delay();
	gpio_set_level(I2C_MASTER_SCL_GPIO,0);//MPU_IIC_SCL=0;
}					 				     
//IIC发送一个字节
//返回从机有无应答
//1，有应答
//0，无应答			  
void MPU_IIC_Send_Byte(uint8_t txd)
{
    uint8_t t;   
	gpio_set_direction(I2C_MASTER_SDA_GPIO,GPIO_MODE_OUTPUT);//MPU_SDA_OUT(); 	    
    gpio_set_level(I2C_MASTER_SCL_GPIO,0);//MPU_IIC_SCL=0;//拉低时钟开始数据传输
    for(t=0;t<8;t++)
    {              
        gpio_set_level(I2C_MASTER_SDA_GPIO,((txd&0x80)>>7));//MPU_IIC_SDA=(txd&0x80)>>7;
        txd<<=1; 	  
		    gpio_set_level(I2C_MASTER_SCL_GPIO,1);//MPU_IIC_SCL=1;
		    MPU_IIC_Delay(); 
		    gpio_set_level(I2C_MASTER_SCL_GPIO,0);//MPU_IIC_SCL=0;	
		    MPU_IIC_Delay();
    }	 
}
//读1个字节，ack=1时，发送ACK，ack=0，发送nACK   
uint8_t MPU_IIC_Read_Byte(unsigned char ack)
{
	unsigned char i,receive=0;
	gpio_set_direction(I2C_MASTER_SDA_GPIO,GPIO_MODE_INPUT);//MPU_SDA_IN();//SDA设置为输入
    for(i=0;i<8;i++ )
	{
        gpio_set_level(I2C_MASTER_SCL_GPIO,0);//MPU_IIC_SCL=0; 
        MPU_IIC_Delay();
		gpio_set_level(I2C_MASTER_SCL_GPIO,1);//MPU_IIC_SCL=1;
        receive<<=1;
        if(gpio_get_level(I2C_MASTER_SDA_GPIO))receive++;   
		MPU_IIC_Delay(); 
    }					 
    if (!ack)
        MPU_IIC_NAck();//发送nACK
    else
        MPU_IIC_Ack(); //发送ACK   
    return receive;
}


























