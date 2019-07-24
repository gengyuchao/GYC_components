# GYC_components DHT11 driver
由我自己开发的ESP-IDF的功能组成部分 DHT11温湿度传感器驱动 

*创建人 gengyuchao

*创建时间 2019/7/24

*内容说明:DHT11温湿度传感器驱动 支持ESP8266_RTOSV3.0库 C++

# 结构目录:

  -component
  
     -dht11_driver                  
         -component.mk
         -CMakeLists.txt
         -include
            -dht11_driver.h
         -library
            dht11_driver.c

# 使用方法

添加工程组件文件夹 dht11_driver 到自己工程的 components （组件） 

在工程中添加头文件、创建传感器设备对象

示例：

```C++
#include "dht11_driver.hpp"
 
//生成 DHT11 的实例化对象

DHT_Sensor my_DHT11(GPIO_NUM_5,DHT11);

//初始化硬件

void DHTInit();

//读取温湿度值

void DHTRead();

//在dht11_driver.cpp文件夹中

//调试步骤打印开关 屏蔽定义 DHT_DEBUG 可以关闭调试信息在控制台的输出

// #define DHT_DEBUG

//数据打印示例
my_DHT11.DHTInit();

my_DHT11.DHTRead();

ESP_LOGI("DHT11","DHT: Temperature*100 =  %d *C, Humidity*100 = %d %% (GPIO%d)",
                (int) (my_DHT11.Sensor_Data.temperature * 100), (int) (my_DHT11.Sensor_Data.humidity * 100), my_DHT11.data_pin);
 
```
