

#ifndef __I2C_PORT_H__
#define __I2C_PORT_H__


#ifdef __cplusplus
extern "C" {
#endif

#include "driver/i2c.h"


// External Wire.h equivalent error Codes
typedef enum {
    I2C_ERROR_OK=0,
    I2C_ERROR_DEV,
    I2C_ERROR_ACK,
    I2C_ERROR_TIMEOUT,
    I2C_ERROR_BUS,
    I2C_ERROR_BUSY,
    I2C_ERROR_MEMORY,
    I2C_ERROR_CONTINUE,
    I2C_ERROR_NO_BEGIN
} i2c_err_t;

// struct i2c_config_t;

typedef struct {
    int num;
    i2c_config_t cfg;
}i2c_t;

unsigned long IRAM_ATTR millis();
void delay(uint32_t ms);

/*
i2c\S*?\(
*/

i2c_t * i2cInit(uint8_t i2c_num, int8_t sda, int8_t scl, uint32_t clk_speed);
void i2cRelease(i2c_t *i2c); // free ISR, Free DQ, Power off peripheral clock.  Must call i2cInit() to recover
i2c_err_t i2cWrite(i2c_t * i2c, uint16_t address, uint8_t* buff, uint16_t size, bool sendStop, uint16_t timeOutMillis);
i2c_err_t i2cRead(i2c_t * i2c, uint16_t address, uint8_t* buff, uint16_t size, bool sendStop, uint16_t timeOutMillis, uint32_t *readCount);
i2c_err_t i2cFlush(i2c_t *i2c);
i2c_err_t i2cSetFrequency(i2c_t * i2c, uint32_t clk_speed);
uint32_t i2cGetFrequency(i2c_t * i2c);
uint32_t i2cGetStatus(i2c_t * i2c); // Status register of peripheral

//stickbreaker debug support
uint32_t i2cDebug(i2c_t *, uint32_t setBits, uint32_t resetBits);

#ifdef __cplusplus
}
#endif

#endif // __I2C_PORT_H__


