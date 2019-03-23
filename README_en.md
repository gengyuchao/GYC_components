# GYC_components
The functional components of ESP-IDF developed by myself

*Founder gengyuchao

*Created time 2019/2/16

*Content Description: Support ESP8266_RTOSV3.0 library

# OLED screen driver and graphics drawing

 Since I often need to use the OLED screen, the driver used on the ESP8266 is slightly organized for use when needed.

 Structure directory:

  -component
  
     -i2c_oled_driver                   #oled driver (optional one)
         -component.mk
         -CMakeLists.txt
         -include
            -i2c_oled_driver.h
         -library
            i2c_oled_driver.c

     -i2c_oled_driver(offical i2c)      #oled driver (optional one)
         -component.mk
         -CMakeLists.txt
         -include
            -i2c_oled_driver.h
         -library
            -i2c_oled_driver.c

     -spi_oled_dirver                   #oled driver (optional one)
         -component.mk
         -CMakeLists.txt
         -include
            -spi_oled_driver.h
         -library
            spi_oled_driver.c

     -oled_display                      #Must
         -component.mk
         -CMakeLists.txt
         -include
            -oled_display.h
            -OLEDDisplayFonts.h
         -library
            -oled_display.c

 Instructions:
 1, oled_display for oled screen drawing, drawing character library
 
 2, i2c_oled, i2c_oled (offical i2c), spi_oled are different ways to drive OLED screens
 
 3, i2c_oled is the i2c driver mode written by using io port simulation, which I have written and modified, and the efficiency is high (no obvious delay after refresh)
 
 4, i2c_oled (offical i2c) is the i2c driver provided by esp_idf as the oled i2c driver is slower than I wrote
 
 5, spi_oled is to use the spi interface provided in esp_idf as the spi driver of oled
 
 
# printf Adding floating point support
 
 file:
 
  -printf with float
    Ets_printf.c
    Ets_sprintf.c
    
 Filef replaces the original ets_printf.c to achieve printf support for floating point numbers. Use the same method as %f in printf in C language.
 Add the ets_sprintf.c file and use ets_sprintf to implement sprintf support for floating point numbers. Use the same method as %f in sprintf in C language.

# ultrasonic sensor drive HC_SR04

  file:

     -driver_HC_SR04
         -component.mk
         -CMakeLists.txt
         -include
             -driver_HC_SR04.h
         -library
            -driver_HC_SR04.c

