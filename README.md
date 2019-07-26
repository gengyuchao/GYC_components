# GYC_components
由我自己开发的ESP-IDF的功能组成部分

*创建人 gengyuchao

*创建时间 2019/2/16

*内容说明:支持ESP8266_RTOSV3.0库

# OLED 屏幕驱动和图形绘制

 由于自己经常需要使用OLED屏幕，所以把在ESP8266上使用的驱动程序稍作整理，方便需要时使用

 ## 结构目录:

    -component
  
     -i2c_oled_driver                   #oled驱动（任选一个）
         -component.mk
         -CMakeLists.txt
         -include
            -i2c_oled_driver.h
         -library
            i2c_oled_driver.c

     -i2c_oled_driver(offical i2c)      #oled驱动（任选一个）
         -component.mk
         -CMakeLists.txt
         -include
            -i2c_oled_driver.h
         -library
            -i2c_oled_driver.c

     -spi_oled_dirver                   #oled驱动（任选一个）
         -component.mk
         -CMakeLists.txt
         -include
            -spi_oled_driver.h
         -library
            spi_oled_driver.c

     -oled_display               #必须
         -component.mk
         -CMakeLists.txt
         -include
            -oled_display.h
            -OLEDDisplayFonts.h
         -library
            -oled_display.c

 使用方法：
 1、oled_display为oled屏幕绘图、绘制字符函数库
 
 2、i2c_oled、i2c_oled(offical i2c)、spi_oled是驱动OLED屏幕的不同方法
 
 3、i2c_oled 是使用io口模拟写的i2c驱动方式，是我自己编写修改的，效率较高（刷新无明显延迟）
 
 4、i2c_oled(offical i2c) 是使用esp_idf中提供的i2c接口作为oled的i2c驱动 效率比我写的要慢
 
 5、spi_oled 是使用esp_idf中提供的spi接口作为oled的spi驱动
 
 
 # printf 添加浮点数支持
 
 ## 结构目录:
 
  -printf with float
    ets_printf.c
    ets_sprintf.c
    
  文件替换掉原有的ets_printf.c 即可实现printf对浮点数的支持。使用方法同C语言中printf中的%f使用方法。
  添加ets_sprintf.c文件，使用ets_sprintf函数 即可实现sprintf对浮点数的支持。使用方法同C语言中sprintf中的%f使用方法。

  
 # 超声波传感器驱动 HC_SR04

 ## 结构目录:

     -driver_HC_SR04
         -component.mk
         -CMakeLists.txt
         -include
             -driver_HC_SR04.h
         -library
            -driver_HC_SR04.c
            
            
 # 三原色灯驱动 WS2812
 
 *博客地址: https://blog.csdn.net/gengyuchao/article/details/93239317
 
 ## 结构目录:

     -ws2812
         -component.mk
         -CMakeLists.txt
         -include
             -ws2812_driver.hpp
         -scr
             -ws2812_driver.cpp
             


 # DHT11 温湿度传感器

 *创建时间 2019/7/24

 *内容说明:DHT11温湿度传感器驱动 支持ESP8266_RTOSV3.0库 C++

 *博客地址: https://blog.csdn.net/gengyuchao/article/details/93239317

 ## 结构目录:

   -component

      -dht11_driver                  
          -component.mk
          -CMakeLists.txt
          -include
             -dht11_driver.h
          -library
             dht11_driver.c

 ## 使用方法

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

