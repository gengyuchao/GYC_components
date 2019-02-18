#include "i2c_oled_driver.h"

#define ESP8266_IIC


#include "driver/i2c.h"
#include "driver/gpio.h"

#define I2C_MASTER_SDA_GPIO OLED_SDA_GPIO
#define I2C_MASTER_SCL_GPIO OLED_SCL_GPIO
 

#define I2C_MASTER_GPIO_OUT(pin,val)  gpio_set_level(pin, val)
// #define delay_us os_delay_us

void delay_us(int xus)//超高速模式,减少延时时间
{
  // volatile uint8_t i;
  // for(i=0;i<xus;i++)
  // {
  //   ;
  // }

}
 
#define WRITE_BIT                           I2C_MASTER_WRITE /*!< I2C master write */
#define READ_BIT                            I2C_MASTER_READ  /*!< I2C master read */
#define ACK_CHECK_EN                        0x1              /*!< I2C master will check ack from slave*/

esp_err_t oled_gpio_init()
{
  #ifndef ESP8266_IIC
    printf("init i2c\n");

    esp_err_t ret ;
    i2c_config_t i2c_conf;

    i2c_conf.mode               = I2C_MODE_MASTER;
    i2c_conf.sda_io_num         = GPIO_NUM_4;
    i2c_conf.sda_pullup_en      = GPIO_PULLUP_ENABLE;
    i2c_conf.scl_io_num         = GPIO_NUM_5;
    i2c_conf.scl_pullup_en      = GPIO_PULLUP_ENABLE;

    // i2c_param_config(I2C_NUM_0, &i2c_conf);
    ret=i2c_set_pin(I2C_NUM_1, i2c_conf.sda_io_num, i2c_conf.scl_io_num,
                                i2c_conf.sda_pullup_en, i2c_conf.scl_pullup_en, i2c_conf.mode);

    printf("init oled");
    printf("\nOLED_SDA_GPIO:%d  OLED_SCL_GPIO:%d",OLED_SDA_GPIO,OLED_SCL_GPIO);
    return ret;

  #else
    int i2c_master_port = I2C_NUM_1;
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = GPIO_NUM_4;
    conf.sda_pullup_en = 0;
    conf.scl_io_num = GPIO_NUM_5;
    conf.scl_pullup_en = 0;
    ESP_ERROR_CHECK(i2c_driver_install(i2c_master_port, conf.mode));
    ESP_ERROR_CHECK(i2c_param_config(i2c_master_port, &conf));
    return ESP_OK;

  #endif
  
}

/*------OLED Base function------*/


esp_err_t oled_delay_ms(uint32_t time)
{
    vTaskDelay(time / portTICK_RATE_MS);
    return ESP_OK;
}


 
//开始信号
static void IIC_Start(void)
{
    gpio_set_direction(I2C_MASTER_SDA_GPIO,GPIO_MODE_OUTPUT);//SDA_OUT();
    I2C_MASTER_GPIO_OUT(I2C_MASTER_SDA_GPIO,1);//IIC_SDA=1;
    I2C_MASTER_GPIO_OUT(I2C_MASTER_SCL_GPIO,1);//IIC_SCL=1;
    delay_us(2);
    I2C_MASTER_GPIO_OUT(I2C_MASTER_SDA_GPIO,0);//IIC_SDA=0;
    delay_us(2);
    I2C_MASTER_GPIO_OUT(I2C_MASTER_SCL_GPIO,0);//IIC_SCL=0;
    delay_us(2);
}
 
static void IIC_Stop(void)
{
    I2C_MASTER_GPIO_OUT(I2C_MASTER_SCL_GPIO,1);//IIC_SCL=1;
    I2C_MASTER_GPIO_OUT(I2C_MASTER_SDA_GPIO,0);//IIC_SDA=0;
    delay_us(2);
    I2C_MASTER_GPIO_OUT(I2C_MASTER_SDA_GPIO,1);//IIC_SDA=1;
    delay_us(2);
}
 
/*
*   返回1--应答出错
*   返回0--应答正确
*/
static uint8_t IIC_Wait_Ask(void)
{
    int count=0;
 
    gpio_set_direction(I2C_MASTER_SDA_GPIO,GPIO_MODE_INPUT);//    SDA_IN();
 
    I2C_MASTER_GPIO_OUT(I2C_MASTER_SCL_GPIO,1);//IIC_SCL=1;
    delay_us(2);
    while(gpio_get_level(I2C_MASTER_SDA_GPIO)) //
    {
        count++;
        if(count>250)
        {
            IIC_Stop();
            return 1;
        }
    }
    I2C_MASTER_GPIO_OUT(I2C_MASTER_SCL_GPIO,0);//IIC_SCL=0;
    delay_us(2);
    return 0;
}
 
//写一个字节
static void IIC_WriteByte(uint8_t data)
{
    uint8_t i;
    gpio_set_direction(I2C_MASTER_SDA_GPIO,GPIO_MODE_OUTPUT);//SDA_OUT();
    for(i=0;i<8;i++)
    {
    	I2C_MASTER_GPIO_OUT(I2C_MASTER_SCL_GPIO,0);//IIC_SCL=0;
        delay_us(2);
        if(data & 0x80)     //MSB,从高位开始一位一位传输
            I2C_MASTER_GPIO_OUT(I2C_MASTER_SDA_GPIO,1);//IIC_SDA=1;
        else
            I2C_MASTER_GPIO_OUT(I2C_MASTER_SDA_GPIO,0);//IIC_SDA=0;
        I2C_MASTER_GPIO_OUT(I2C_MASTER_SCL_GPIO,1);//IIC_SCL=1;
        delay_us(2);
        I2C_MASTER_GPIO_OUT(I2C_MASTER_SCL_GPIO,0);//IIC_SCL=0;
        data<<=1;
 
    }
}
 
 
static uint8_t IIC_ReadByte(void)
{
    uint8_t data=0,i=0;
    I2C_MASTER_GPIO_OUT(I2C_MASTER_SDA_GPIO,1);//IIC_SDA=1;
    delay_us(2);
    for(i=0;i<8;i++)
    {
        data<<=1;
        I2C_MASTER_GPIO_OUT(I2C_MASTER_SCL_GPIO,0);//IIC_SCL=0;
        delay_us(2);
        I2C_MASTER_GPIO_OUT(I2C_MASTER_SCL_GPIO,1);//IIC_SCL=1;
        delay_us(2);
        if(gpio_get_level(I2C_MASTER_SDA_GPIO))//(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_7))
            data=data | 0x01;
        else
            data=data & 0xFE;
 
    }
    I2C_MASTER_GPIO_OUT(I2C_MASTER_SCL_GPIO,0);//IIC_SCL=0;
    delay_us(2);
    return data;
 
}



esp_err_t oled_write_cmd(uint8_t command)
{
  #ifndef ESP8266_IIC
    IIC_Start();
    IIC_WriteByte(0x78);//OLED地址
    IIC_Wait_Ask();
    IIC_WriteByte(0x00);//寄存器地址
    IIC_Wait_Ask();
    IIC_WriteByte(command);
    IIC_Wait_Ask();
    IIC_Stop();
    return ESP_OK;
  #else
    int ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, 0x78 , ACK_CHECK_EN);
    i2c_master_write_byte(cmd, 0x00 , ACK_CHECK_EN);
    i2c_master_write_byte(cmd, command              , ACK_CHECK_EN);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_NUM_1, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    return ret;
  #endif
}
 
 
esp_err_t oled_write_byte(uint8_t data)
{
  #ifndef ESP8266_IIC
    IIC_Start();
    IIC_WriteByte(0x78);//OLED地址
    IIC_Wait_Ask();
    IIC_WriteByte(0x40);//寄存器地址
    IIC_Wait_Ask();
    IIC_WriteByte(data);
    IIC_Wait_Ask();
    IIC_Stop();
    return ESP_OK;
  #else
    int ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, 0x78 , ACK_CHECK_EN);
    i2c_master_write_byte(cmd, 0x40 , ACK_CHECK_EN);
    i2c_master_write_byte(cmd, data , ACK_CHECK_EN);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_NUM_1, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    return ret;
  #endif
}

esp_err_t oled_write_data(uint8_t *data,int length)
{
  #ifndef ESP8266_IIC
    IIC_Start();
    IIC_WriteByte(0x78);//OLED地址
    IIC_Wait_Ask();
    IIC_WriteByte(0x40);//寄存器地址
    IIC_Wait_Ask();
    for(int i=0;i<length;i++)
    {
      IIC_WriteByte(*(data+i));
      IIC_Wait_Ask();
    }
    IIC_Stop();
    return ESP_OK;
  #else
    int ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, 0x78 , ACK_CHECK_EN);
    i2c_master_write_byte(cmd, 0x40 , ACK_CHECK_EN);
    for(int i=0;i<length;i++)
      i2c_master_write_byte(cmd, *(data + i) , ACK_CHECK_EN);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_NUM_1, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    return ret;
  #endif
}
