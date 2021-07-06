#ifdef __cplusplus
extern "C" {
#endif


#include "esp_err.h"
#include "esp_intr_alloc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/ringbuf.h"
// #include "driver/gpio.h"
// #include "soc/soc_caps.h"
// #include "hal/i2c_types.h"
// #include "hal/i2c_hal.h"

#include "driver/i2c.h"
#include "i2c_port.h"

#define WRITE_BIT I2C_MASTER_WRITE              /*!< I2C master write */
#define READ_BIT I2C_MASTER_READ  

#define ACK_CHECK_EN 0x1                        /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS 0x0                       /*!< I2C master will not check ack from slave */
#define ACK_VAL 0x0                             /*!< I2C ack value */
#define NACK_VAL 0x1   

#define I2C_MASTER_TX_BUF_DISABLE 0                           /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0   

unsigned long IRAM_ATTR micros()
{
    return (unsigned long) (esp_timer_get_time());
}

unsigned long IRAM_ATTR millis()
{
    return (unsigned long) (esp_timer_get_time() / 1000ULL);
}

void delay(uint32_t ms)
{
    vTaskDelay(ms / portTICK_PERIOD_MS);
}



i2c_err_t i2cInit(uint8_t i2c_num, int8_t sda, int8_t scl, uint32_t clk_speed)
{
    int i2c_master_port = i2c_num;
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = sda,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = scl,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = clk_speed,
        // .clk_flags = 0,          /*!< Optional, you can use I2C_SCLK_SRC_FLAG_* flags to choose i2c source clock here. */
    };
    esp_err_t err = i2c_param_config(i2c_master_port, &conf);
    if (err != ESP_OK) {
        return err;
    }
    return i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);

}
void i2cRelease(i2c_t i2c)
{
    ;
} // free ISR, Free DQ, Power off peripheral clock.  Must call i2cInit() to recover
i2c_err_t i2cWrite(i2c_t i2c_num, uint16_t address, uint8_t* buff, uint16_t size, bool sendStop, uint16_t timeOutMillis)
{
    #if 0
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (address << 1) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write(cmd, buff, size, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, timeOutMillis);
    i2c_cmd_link_delete(cmd);
    return ret;
    #endif
    if(sendStop == false) {
        assert(0); //Don't sendStop not support now.
    }
    return i2c_master_write_to_device(i2c_num,address,buff,size,timeOutMillis);
}
i2c_err_t i2cRead(i2c_t i2c_num, uint16_t address, uint8_t* buff, uint16_t size, bool sendStop, uint16_t timeOutMillis, uint32_t *readCount)
{
    #if 0
    if (size == 0) {
        return ESP_OK;
    }
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (address << 1) | READ_BIT, ACK_CHECK_EN);
    if (size > 1) {
        i2c_master_read(cmd, buff, size - 1, ACK_VAL);
    }
    if(sendStop) {
        i2c_master_read_byte(cmd, buff + size - 1, NACK_VAL);
        i2c_master_stop(cmd);        
    }

    esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, timeOutMillis);
    i2c_cmd_link_delete(cmd);
    return ret;
    #endif
    if(sendStop == false) {
        assert(0); //Don't sendStop not support now.
    }
    return i2c_master_read_from_device(i2c_num,address,buff,size,timeOutMillis);
}
i2c_err_t i2cFlush(i2c_t i2c_num)
{
    // i2c_param_config(i2c_num, const i2c_config_t *i2c_conf);
    return I2C_ERROR_OK;
}
i2c_err_t i2cSetFrequency(i2c_t i2c, uint32_t clk_speed)
{
    return I2C_ERROR_OK;
}
uint32_t i2cGetFrequency(i2c_t i2c)
{
    return I2C_ERROR_OK;
}
uint32_t i2cGetStatus(i2c_t i2c)
{
    return i2c;
} // Status register of peripheral



uint32_t i2cDebug(i2c_t i2c, uint32_t setBits, uint32_t resetBits){
    // if(i2c != NULL) {
    //     i2c->debugFlags = ((i2c->debugFlags | setBits) & ~resetBits);
    //     return i2c->debugFlags;
    // }
    // return 0;
    return I2C_ERROR_OK;
 }

#ifdef __cplusplus
}
#endif
