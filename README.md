# GYC_components
由我自己开发的ESP-IDF的功能组成部分

*创建人 gengyuchao

*创建时间 2019/2/16

*内容说明:支持ESP8266_RTOSV3.0库

# OLED 屏幕驱动和图形绘制

 由于自己经常需要使用OLED屏幕，所以把在ESP8266上使用的驱动程序稍作整理，方便需要时使用

 结构目录:

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
 
 文件：
 
  -printf with float
    ets_printf.c
    ets_sprintf.c
    
  文件替换掉原有的ets_printf.c 即可实现printf对浮点数的支持。使用方法同C语言中printf中的%f使用方法。
  添加ets_sprintf.c文件，使用ets_sprintf函数 即可实现sprintf对浮点数的支持。使用方法同C语言中sprintf中的%f使用方法。

  
 # 超声波传感器驱动 HC_SR04

 文件：

     -driver_HC_SR04
         -component.mk
         -CMakeLists.txt
         -include
             -driver_HC_SR04.h
         -library
            -driver_HC_SR04.c

