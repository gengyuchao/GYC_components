#创建人 gengyuchao
#创建时间 2019/2/16
#内容说明:

# 由于自己经常需要使用OLED屏幕，所以把在ESP8266上使用的驱动程序稍作整理，方便需要时使用

# 结构目录:

# -component
#     -i2c_oled                   #oled驱动（任选一个）
#         -README.md
#         -component.mk
#         -CMakeLists.txt
#         -include
#         -library

#     -i2c_oled(offical i2c)      #oled驱动（任选一个）
#         -README.md
#         -component.mk
#         -CMakeLists.txt
#         -include
#         -library

#     -spi_oled                   #oled驱动（任选一个）
#         -README.md
#         -component.mk
#         -CMakeLists.txt
#         -include
#         -library

#     -oled_display               #必须
#         -README.md
#         -component.mk
#         -CMakeLists.txt
#         -include
#         -library

# 使用方法：
# 1、oled_display为oled屏幕绘图、绘制字符函数库
# 2、i2c_oled、i2c_oled(offical i2c)、spi_oled是驱动OLED屏幕的不同方法
# 3、i2c_oled 是使用io口模拟写的i2c驱动方式，是我自己编写修改的，效率较高（刷新无明显延迟）
# 4、i2c_oled(offical i2c) 是使用esp_idf中提供的i2c接口作为oled的i2c驱动 效率比我写的要慢
# 5、spi_oled 是使用esp_idf中提供的spi接口作为oled的spi驱动