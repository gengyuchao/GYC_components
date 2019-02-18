#Created_By_gengyuchao
#
#Created Time 2019/2/16
#
#Content Description:

#Because I often need to use the OLED screen, the driver used on ESP8266 is slightly organized so that it can be used when needed.

#Structure catalog:

# -component
#     -i2c_oled                   #oled Drive (optional one)
#         -README.md
#         -component.mk
#         -CMakeLists.txt
#         -include
#         -library

#     -i2c_oled(offical i2c)      #oled Drive (optional one)
#         -README.md
#         -component.mk
#         -CMakeLists.txt
#         -include
#         -library

#     -spi_oled                   #oled Drive (optional one)
#         -README.md
#         -component.mk
#         -CMakeLists.txt
#         -include
#         -library

#     -oled_display               #MUST
#         -README.md
#         -component.mk
#         -CMakeLists.txt
#         -include
#         -library

#Instructions:
#1,oled_displayDraw for oled screen, draw character library
#2,i2c_oled,i2c_oled (official i2c), spi_oled is a different way to drive OLED screens
#3,i2c_oled is the i2c driver that uses the io port to simulate writing. It is my own edited and modified, and the efficiency is high (no significant delay in refreshing)
#4,i2c_oled(offical i2c) is the i2c driver that uses the i2c interface provided in esp_idf as oled. The efficiency is slower than what I wrote.
#5,spi_oled is to use the spi interface provided in esp_idf as the spi driver for oled