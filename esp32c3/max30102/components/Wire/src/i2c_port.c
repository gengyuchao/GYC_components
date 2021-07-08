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


#include "driver/i2c.h"
#include "hal/i2c_hal.h"
#include "i2c_port.h"

#include "string.h"

#include "esp_log.h"

#define TAG "I2C_PORT"

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



i2c_t * i2cInit(uint8_t i2c_num, int8_t sda, int8_t scl, uint32_t clk_speed){

    esp_err_t err = ESP_OK;
    i2c_t *i2c = NULL;

    if(i2c_num > 1) {
        err = ESP_ERR_INVALID_ARG;
        goto error;
    }

    i2c = malloc(sizeof(i2c_t));
    if(i2c == NULL) {
        err = ESP_ERR_NO_MEM;
        goto error;
    }

    memset(i2c,0,sizeof(i2c_t));
    i2c->num = i2c_num;
    i2c->cfg.mode = I2C_MODE_MASTER,
    i2c->cfg.sda_io_num = sda,
    i2c->cfg.sda_pullup_en = GPIO_PULLUP_ENABLE,
    i2c->cfg.scl_io_num = scl,
    i2c->cfg.scl_pullup_en = GPIO_PULLUP_ENABLE,
    i2c->cfg.master.clk_speed = clk_speed,
    // i2c->cfg.clk_flags = 0,          /*!< Optional, you can use I2C_SCLK_SRC_FLAG_* flags to choose i2c source clock here. */

    err = i2c_param_config(i2c->num, &i2c->cfg);
    if (err != ESP_OK) {
        goto error;
    }
    err = i2c_driver_install(i2c->num, i2c->cfg.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
    if (err != ESP_OK) {    
        goto error;   
    }
    ESP_LOGI(TAG,"%s:init success!SCL:%d,SDA:%d\n",__func__,i2c->cfg.scl_io_num,i2c->cfg.sda_io_num);
    return i2c;
error:
    ESP_LOGE(TAG,"%s:%d\n",__func__,esp_err_to_name(err));
    if(i2c != NULL) {
        free(i2c);
        i2c = NULL;
    }
    return NULL;
}

void i2cRelease(i2c_t *i2c){// free ISR, Free DQ, Power off peripheral clock.  Must call i2cInit() to recover
    i2c_driver_delete(i2c->num);
    free(i2c);
}



#define I2C_TRANS_BUF_MINIMUM_SIZE     (1024) /* It is required to have allocate one i2c_cmd_desc_t per command:
                                                                     * start + write (device address) + write buffer +
                                                                     * start + write (device address) + read buffer + read buffer for NACK +
                                                                     * stop */

i2c_err_t i2cWrite(i2c_t * i2c, uint16_t device_address, uint8_t* write_buffer, uint16_t write_size, bool sendStop, uint16_t ticks_to_wait){
    if((i2c==NULL)||((write_size>0)&&(write_buffer==NULL))) { // need to have location to store requested data
        return I2C_ERROR_DEV;
    }
    esp_err_t err = ESP_OK;
    i2c_err_t i2c_err = I2C_ERROR_OK;
    uint8_t buffer[I2C_TRANS_BUF_MINIMUM_SIZE] = { 0 };

    i2c_cmd_handle_t handle = i2c_cmd_link_create_static(buffer, sizeof(buffer));
    assert (handle != NULL);

    err = i2c_master_start(handle);
    if (err != ESP_OK) {
        i2c_err = I2C_ERROR_MEMORY;
        goto end;
    }

    err = i2c_master_write_byte(handle, device_address << 1 | I2C_MASTER_WRITE, true);
    if (err != ESP_OK) {
        i2c_err = I2C_ERROR_MEMORY;
        goto end;
    }

    err = i2c_master_write(handle, write_buffer, write_size, true);
    if (err != ESP_OK) {
        i2c_err = I2C_ERROR_MEMORY;
        goto end;
    }

    if(sendStop) {
        err = i2c_master_stop(handle);
        if (err != ESP_OK) {
            i2c_err = I2C_ERROR_MEMORY;
            goto end;
        }   
    } else {
        i2c_err = I2C_ERROR_CONTINUE;
    }
    err = i2c_master_cmd_begin(i2c->num, handle, ticks_to_wait);

end:
    i2c_cmd_link_delete_static(handle);
    if(err == ESP_ERR_TIMEOUT) {
        i2c_err = I2C_ERROR_TIMEOUT;
    } else if (err != ESP_OK){
        i2c_err = I2C_ERROR_DEV;
    }
    return i2c_err;
}

i2c_err_t i2cRead(i2c_t * i2c, uint16_t device_address, uint8_t* read_buffer, uint16_t read_size, bool sendStop, uint16_t ticks_to_wait, uint32_t *readCount){

    if((read_size == 0)||(i2c == NULL)||(read_buffer==NULL)){ // hardware will hang if no data requested on READ
        return I2C_ERROR_DEV;
    }

    esp_err_t err = ESP_OK;
    i2c_err_t i2c_err = I2C_ERROR_OK;
    uint8_t buffer[I2C_TRANS_BUF_MINIMUM_SIZE] = { 0 };

    if(readCount) {
        *readCount = 0;
    }

    i2c_cmd_handle_t handle = i2c_cmd_link_create_static(buffer, sizeof(buffer));
    assert (handle != NULL);

    err = i2c_master_start(handle);
    if (err != ESP_OK) {
        i2c_err = I2C_ERROR_MEMORY;
        goto end;
    }

    err = i2c_master_write_byte(handle, device_address << 1 | I2C_MASTER_READ, true);
    if (err != ESP_OK) {
        i2c_err = I2C_ERROR_MEMORY;
        goto end;
    }

    err = i2c_master_read(handle, read_buffer, read_size, I2C_MASTER_LAST_NACK);
    if (err != ESP_OK) {
        i2c_err = I2C_ERROR_MEMORY;
        goto end;
    }
    if (sendStop) {
        err = i2c_master_stop(handle);
        if (err != ESP_OK) {
            i2c_err = I2C_ERROR_MEMORY;
            goto end;
        }
    } else {
        i2c_err = I2C_ERROR_CONTINUE;
    }
    
    err = i2c_master_cmd_begin(i2c->num, handle, ticks_to_wait);

end:
    i2c_cmd_link_delete_static(handle);

    if(readCount) {
        *readCount = read_size;
    }
    if(err == ESP_ERR_TIMEOUT) {
        i2c_err = I2C_ERROR_TIMEOUT;
    } else if (err != ESP_OK){
        i2c_err = I2C_ERROR_DEV;
    }
    return i2c_err;
}
esp_err_t IRAM_ATTR i2c_hw_fsm_reset(i2c_port_t i2c_num);
i2c_err_t i2cFlush(i2c_t *i2c){

    // return i2c_hw_fsm_reset(i2c->num);
    return I2C_ERROR_OK;
}

i2c_err_t i2cSetFrequency(i2c_t * i2c, uint32_t clk_speed){
    i2c->cfg.master.clk_speed = clk_speed;
    
    return i2c_param_config(i2c->num,&i2c->cfg);
}

uint32_t i2cGetFrequency(i2c_t * i2c){
    return i2c->cfg.master.clk_speed;
}

// uint32_t i2cGetStatus(i2c_t * i2c){// Status register of peripheral
//     return i2c->cfg.;
// } 

//stickbreaker debug support
uint32_t i2cDebug(i2c_t *i2c, uint32_t setBits, uint32_t resetBits){
    return ESP_OK;
}




#ifdef __cplusplus
}
#endif
