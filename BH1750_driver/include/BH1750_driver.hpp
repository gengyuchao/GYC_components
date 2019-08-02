/*
  This example code is created by gengyuchao used for Latest ESP8266 
  SDK based on FreeRTOS, esp-idf style.  (update:2019-08-01).
  Unless required by applicable law or agreed to in writing, this
  software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
  CONDITIONS OF ANY KIND, either express or implied.
 */

#ifndef __BH1750_DEVICE_HPP_
#define __BH1750_DEVICE_HPP_

    #include "esp_system.h"
    #include <string.h>
    #include "esp_log.h"    

    #include "freertos/FreeRTOS.h"
    #include "freertos/task.h"
    #include "driver/i2c.h"

    enum BH1750_MODE{
        BH1750_FAST_MODE,      //快速模式
        BH1750_ACCURATE_MODE   //精确模式
    };


    class IIC_Device
    {

        private:

        gpio_num_t sda_io_num; //I2C_MASTER_SDA_GPIO
        gpio_num_t scl_io_num; //I2C_MASTER_SCL_GPIO

        public:
        IIC_Device(gpio_num_t sda_io, gpio_num_t scl_io)
        :sda_io_num(sda_io),scl_io_num(scl_io)
        {
            gpio_init(sda_io_num,scl_io_num);
        }

        /*
        *   IIC GPIO初始化函数
        *   参数：sda_io_num SDA引脚，scl_io_num SCL引脚
        *   返回结果 ：成功
        */
        esp_err_t gpio_init(gpio_num_t sda_io, gpio_num_t scl_io);

        protected:
        
        void    IIC_Start(void);    //IIC 开始信号
        void    IIC_Stop(void);     //IIC 结束信号

        /*
        *   IIC等待应答函数
        *   返回1--应答出错
        *   返回0--应答正确
        */
        uint8_t IIC_Wait_Ask(void);        

        /*
        *   写一个字节
        *   参数：要写入的数据
        */
        void    IIC_WriteByte(uint8_t data);

        /*
        *   读一个字节
        *   返回值：读出的字节
        */
        uint8_t IIC_ReadByte(void);

        /*
        *   发送Ack 应答信号
        *   参数：是否应答 1->NOACK  0->Ack
        */
        void    SendACK(uint8_t ack);

    };

    class BH1750_Device : public IIC_Device
    {

    private:
        uint8_t SlaveAddress = 0x46; //定义器件在IIC总线中的从地址,根据ALT  ADDRESS地址引脚不同修改
                                     //ALT  ADDRESS引脚接地时地址为0xA6，接电源时地址为0x3A

        uint8_t BUF[8] = {0, 0, 0, 0, 0, 0, 0, 0}; //接收数据缓存区

        /**
         *   通过IIC向BH1750发送数据
         */
        void BH1750_SendByte(uint8_t data);

        /**
         *   通过IIC读取BH1750数据
         */
        uint8_t BH1750_RecvByte();

        /**
         *   向BH1750目标地址写数据
         *   参数：目标地址
         */
        void Single_Write_BH1750(uint8_t REG_Address);
        
        /**
         *   连续读出BH1750内部数据
         */
        void Multiple_Read_BH1750(void);

    public:
        float data = 0;

        /**
         *   BH1750运行的精度模式
         */
        BH1750_MODE currect_mode = BH1750_FAST_MODE;

        BH1750_Device(gpio_num_t sda_io_num, gpio_num_t scl_io_num) : IIC_Device(sda_io_num, scl_io_num)
        {
            init();
        }

        /**
         *  初始化BH1750，根据需要请参考pdf进行修改****
         */
        void init();

        /**
         *   读取BH1750传感器数据
         */
        float read_data();

        /**
         *  设置BH1750 的精度模式 
         */
        void set_mode(BH1750_MODE mode);
        esp_err_t delay_ms(uint32_t time);
    };

#endif


