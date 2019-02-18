#include "i2c_oled_driver.h"

#include "driver/i2c.h"


#define I2C_MASTER_SDA_GPIO OLED_SDA_GPIO
#define I2C_MASTER_SCL_GPIO OLED_SCL_GPIO
 

#define I2C_MASTER_GPIO_OUT(pin,val)  gpio_set_level(pin, val)

// #define delay_us os_delay_us //慢速模式，确保I2C稳定
static void delay_us(int xus)//超高速模式,减少延时时间
{
  // volatile uint8_t i;
  // for(i=0;i<xus;i++);
}

esp_err_t oled_delay_ms(uint32_t time)
{
    vTaskDelay(time / portTICK_RATE_MS);
    return ESP_OK;
}

esp_err_t oled_gpio_init(/*int sda_io_num, int scl_io_num*/)
{

    int sda_io_num = OLED_SDA_GPIO;
    int scl_io_num = OLED_SCL_GPIO;
    gpio_config_t io_conf;

    printf("init oled i2c\n");
    // disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    // set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT_OD;
    // bit mask of the pins that you want to set
    io_conf.pin_bit_mask = (1ULL << sda_io_num)|(1ULL << scl_io_num);
    // disable pull-down mode
    io_conf.pull_down_en = 0;
    // disable pull-up mode
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    // configure GPIO with the given settings
    ESP_ERROR_CHECK(gpio_config(&io_conf));
    ESP_ERROR_CHECK(gpio_set_level(sda_io_num, 1));
    ESP_ERROR_CHECK(gpio_set_level(scl_io_num, 1));

    printf("\nOLED_SDA_GPIO:%d  OLED_SCL_GPIO:%d",sda_io_num,scl_io_num);

    return ESP_OK;
}

/*------IIC Base function------*/

 
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
  IIC_Start();
  IIC_WriteByte(0x78);//OLED地址
  IIC_Wait_Ask();
  IIC_WriteByte(0x00);//寄存器地址
  IIC_Wait_Ask();
  IIC_WriteByte(command);
  IIC_Wait_Ask();
  IIC_Stop();
  return ESP_OK;
}
 
esp_err_t oled_write_byte(uint8_t data)
{

  IIC_Start();
  IIC_WriteByte(0x78);//OLED地址
  IIC_Wait_Ask();
  IIC_WriteByte(0x40);//寄存器地址
  IIC_Wait_Ask();
  IIC_WriteByte(data);
  IIC_Wait_Ask();
  IIC_Stop();
  return ESP_OK;

}

esp_err_t oled_write_data(uint8_t *data,int length)
{

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

}
