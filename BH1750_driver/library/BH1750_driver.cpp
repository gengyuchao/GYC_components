/*
  This example code is created by gengyuchao used for Latest ESP8266 
  SDK based on FreeRTOS, esp-idf style.  (update:2019-08-01).
  Unless required by applicable law or agreed to in writing, this
  software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
  CONDITIONS OF ANY KIND, either express or implied.
 */



#include "BH1750_driver.hpp"

static const char *TAG = "BH1750 driver";

#define I2C_MASTER_GPIO_OUT(pin,val)  gpio_set_level(pin, val)

#define delay_us os_delay_us //慢速模式，确保I2C稳定


esp_err_t BH1750_Device::delay_ms(uint32_t time)
{
    vTaskDelay(time / portTICK_RATE_MS);
    return ESP_OK;
}

/*
 *   IIC GPIO初始化函数
 *   参数：sda_io_num SDA引脚，scl_io_num SCL引脚
 *   返回结果 ：成功
 */
esp_err_t IIC_Device::gpio_init(gpio_num_t sda_io_num, gpio_num_t scl_io_num)
{
    gpio_config_t io_conf;

    printf("init BH1750 i2c\n");
    // disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    // set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT_OD;
    // bit mask of the pins that you want to set
    io_conf.pin_bit_mask = (1ULL << sda_io_num) | (1ULL << scl_io_num);
    // disable pull-down mode
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    // disable pull-up mode
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    // configure GPIO with the given settings
    ESP_ERROR_CHECK(gpio_config(&io_conf));
    ESP_ERROR_CHECK(gpio_set_level(sda_io_num, 1));
    ESP_ERROR_CHECK(gpio_set_level(scl_io_num, 1));

    printf("\nBH1750_SDA_GPIO:%d  BH1750_SCL_GPIO:%d", sda_io_num, scl_io_num);

    return ESP_OK;
}

/*------IIC Base function------*/

/* 
 *   IIC 开始信号
 */
void IIC_Device::IIC_Start(void)
{
    gpio_set_direction(sda_io_num, GPIO_MODE_OUTPUT);   //SDA_OUT();
    I2C_MASTER_GPIO_OUT(sda_io_num, 1);                 //IIC_SDA=1;
    I2C_MASTER_GPIO_OUT(scl_io_num, 1);                 //IIC_SCL=1;
    delay_us(2);
    I2C_MASTER_GPIO_OUT(sda_io_num, 0);                 //IIC_SDA=0;
    delay_us(2);
    I2C_MASTER_GPIO_OUT(scl_io_num, 0);                 //IIC_SCL=0;
    delay_us(2);
}

/* 
 *   IIC 结束信号
 */
void IIC_Device::IIC_Stop(void)
{
    I2C_MASTER_GPIO_OUT(scl_io_num, 1);                 //IIC_SCL=1;
    I2C_MASTER_GPIO_OUT(sda_io_num, 0);                 //IIC_SDA=0;
    delay_us(2);
    I2C_MASTER_GPIO_OUT(sda_io_num, 1);                 //IIC_SDA=1;
    delay_us(2);
}

/*
 *   IIC等待应答函数
 *   返回1--应答出错
 *   返回0--应答正确
 */
uint8_t IIC_Device::IIC_Wait_Ask(void)
{
    int count = 0;

    gpio_set_direction(sda_io_num, GPIO_MODE_INPUT);    //SDA_IN();

    I2C_MASTER_GPIO_OUT(scl_io_num, 1);                 //IIC_SCL=1;
    delay_us(2);
    while (gpio_get_level(sda_io_num)) //
    {
        count++;
        if (count > 250)
        {
            IIC_Stop();
            return 1;
        }
    }
    I2C_MASTER_GPIO_OUT(scl_io_num, 0);                 //IIC_SCL=0;
    delay_us(2);
    return 0;
}

/*
 *   写一个字节
 *   参数：要写入的数据
 */
void IIC_Device::IIC_WriteByte(uint8_t data)
{
    uint8_t i;
    gpio_set_direction(sda_io_num, GPIO_MODE_OUTPUT);   //SDA_OUT();
    for (i = 0; i < 8; i++)
    {
        I2C_MASTER_GPIO_OUT(scl_io_num, 0);             //IIC_SCL=0;
        delay_us(2);
        if (data & 0x80)                                //MSB,从高位开始一位一位传输
            I2C_MASTER_GPIO_OUT(sda_io_num, 1);         //IIC_SDA=1;
        else
            I2C_MASTER_GPIO_OUT(sda_io_num, 0);         //IIC_SDA=0;
        I2C_MASTER_GPIO_OUT(scl_io_num, 1);             //IIC_SCL=1;
        delay_us(2);
        I2C_MASTER_GPIO_OUT(scl_io_num, 0);             //IIC_SCL=0;
        data <<= 1;
    }
}

/*
 *   读一个字节
 *   返回值：读出的字节
 */
uint8_t IIC_Device::IIC_ReadByte(void)
{
    uint8_t data = 0, i = 0;
    I2C_MASTER_GPIO_OUT(sda_io_num, 1);                 //IIC_SDA=1;
    delay_us(2);
    gpio_set_direction(sda_io_num, GPIO_MODE_INPUT);    //SDA_OUT();
    for (i = 0; i < 8; i++)
    {
        data <<= 1;
        I2C_MASTER_GPIO_OUT(scl_io_num, 0);             //IIC_SCL=0;
        delay_us(2);
        I2C_MASTER_GPIO_OUT(scl_io_num, 1);             //IIC_SCL=1;
        delay_us(2);
        if (gpio_get_level(sda_io_num))                 //
            data = data | 0x01;
        else
            data = data & 0xFE;
    }
    I2C_MASTER_GPIO_OUT(scl_io_num, 0);                 //IIC_SCL=0;
    delay_us(2);
    return data;
}

/*
 *   发送Ack 应答信号
 *   参数：是否应答 1->NOACK  0->Ack
 */
void IIC_Device::SendACK(uint8_t ack)
{
    gpio_set_direction(sda_io_num, GPIO_MODE_OUTPUT);   //MPU_SDA_OUT();
    gpio_set_level(scl_io_num, 0);                      //MPU_IIC_SCL=0;
    I2C_MASTER_GPIO_OUT(sda_io_num, ack);               //SDA = ack;                  //写应答信号
    I2C_MASTER_GPIO_OUT(scl_io_num, 1);                 //SCL = 1;                    //拉高时钟线
    delay_us(2);                                        //延时
    I2C_MASTER_GPIO_OUT(scl_io_num, 0);                 //SCL = 0;                    //拉低时钟线
    delay_us(2);                                        //延时
}



/**
 *   通过IIC向BH1750发送数据
 */
void BH1750_Device::BH1750_SendByte(uint8_t data)
{
    IIC_WriteByte(data);
    IIC_Wait_Ask();
}

/**
 *   通过IIC读取BH1750数据
 */
uint8_t BH1750_Device::BH1750_RecvByte()
{
    return IIC_ReadByte();
}


/**
 *   向BH1750目标地址写数据
 *   参数：目标地址
 */
void BH1750_Device::Single_Write_BH1750(uint8_t REG_Address)
{
    IIC_Start();                    //起始信号
    BH1750_SendByte(SlaveAddress);  //发送设备地址+写信号
    BH1750_SendByte(REG_Address);   //内部寄存器地址，请参考中文pdf22页
    //BH1750_SendByte(REG_data);    //内部寄存器数据，请参考中文pdf22页
    IIC_Stop();                     //发送停止信号
}


/**
 *   连续读出BH1750内部数据
 */
void BH1750_Device::Multiple_Read_BH1750(void)
{
    uint8_t i;
    IIC_Start();                            //起始信号
    BH1750_SendByte(SlaveAddress | 0x01);   //发送设备地址+读信号

    for (i = 0; i < 3; i++)                 //连续读取6个地址数据，存储中BUF
    {
        BUF[i] = BH1750_RecvByte();         //BUF[0]存储0x32地址中的数据
        if (i == 3)
        {
            SendACK(1);                     //最后一个数据需要回NOACK
        }
        else
        {
            SendACK(0);                     //回应ACK
        }
    }

    IIC_Stop(); //停止信号
    delay_ms(5);
}

/**
 *  初始化BH1750，根据需要请参考pdf进行修改****
 */
void BH1750_Device::init()
{
    delay_ms(10);
    Single_Write_BH1750(0x01);  
}

/**
 *  设置BH1750 的精度模式 
 */
void BH1750_Device::set_mode(BH1750_MODE mode)
{
    currect_mode = mode;
}

/**
 *   读取BH1750传感器数据
 */
float BH1750_Device::read_data()
{
    float   temp = 0;
    int     dis_data = 0; //变量

    if (currect_mode == BH1750_FAST_MODE)
    {
        Single_Write_BH1750(0x01); // power on
        Single_Write_BH1750(0x13); // L- resolution mode
        delay_ms(18);
    }
    else if (currect_mode == BH1750_ACCURATE_MODE)
    {
        Single_Write_BH1750(0x01); // power on
        Single_Write_BH1750(0x10); // H- resolution mode
        delay_ms(180);             //延时180ms
    }

    Multiple_Read_BH1750(); //连续读出数据，存储在BUF中

    //printf("BUF = %d,%d,%d,%d,%d,%d,%d,%d",BUF[0],BUF[1],BUF[2],BUF[3],BUF[4],BUF[5],BUF[6],BUF[7]);

    dis_data = BUF[0];
    dis_data = (dis_data << 8) + BUF[1]; //合成数据

    temp = (float)dis_data / 1.2;
    data = temp;
    return temp;
}

