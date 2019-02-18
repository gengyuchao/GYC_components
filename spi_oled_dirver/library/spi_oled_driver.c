#include "oled_display.h"

#include "spi_oled_driver.h"

static const char *TAG = "spi_oled_driver";

/*------OLED Base function------*/

static uint8_t oled_dc_level = 0;

esp_err_t oled_delay_ms(uint32_t time)
{
    vTaskDelay(time / portTICK_RATE_MS);
    return ESP_OK;
}

esp_err_t oled_set_dc(uint8_t dc)
{
    oled_dc_level = dc;
    return ESP_OK;
}

// Write an 8-bit cmd
esp_err_t oled_write_cmd(uint8_t data)
{
    uint32_t buf = data << 24;
    spi_trans_t trans = {0};
    trans.mosi = &buf;
    trans.bits.mosi = 8;
    oled_set_dc(0);
    spi_trans(HSPI_HOST, trans);
    return ESP_OK;
}

// Write an 8-bit data
esp_err_t oled_write_byte(uint8_t data)
{
    uint32_t buf = data << 24;
    spi_trans_t trans = {0};
    trans.mosi = &buf;
    trans.bits.mosi = 8;
    oled_set_dc(1);
    spi_trans(HSPI_HOST, trans);
    return ESP_OK;
}

// Write many-bit data
esp_err_t oled_write_data(uint8_t *data,int length)
{
    uint8_t* data_pos=data;

    while(length>=64)
    {
      uint8_t x={0};
      uint32_t buf[16]={0};
      spi_trans_t trans = {0};
      trans.mosi = buf;
      trans.bits.mosi = 64 * 8;

      for (x = 0; x < 16 ; x++) {
          buf[x] = *data_pos << 24 | *(data_pos+1) << 16 | *(data_pos+2) << 8 | *(data_pos+3);
          data_pos+=4;
      }

      // SPI transfers 64 bytes at a time, transmits many times, increasing the screen refresh rate

      oled_set_dc(1);
      spi_trans(HSPI_HOST, trans);

      length-=64;
    }



    if(length>0 && length<64)//MAX size in one times
    {

      uint8_t x;
      uint32_t buf[16];
      spi_trans_t trans = {0};
      trans.mosi = buf;
      trans.bits.mosi = length * 8;

      for (x = 0; x <= length/4 ; x++) {
          buf[x] = *data_pos << 24 | *(data_pos+1) << 16 | *(data_pos+2) << 8 | *(data_pos+3);
          data_pos+=4;
      }

      // SPI transfers last bytes at a time, increasing the screen refresh rate

      oled_set_dc(1);
      spi_trans(HSPI_HOST, trans);

    }

    return ESP_OK;
}

esp_err_t oled_rst()
{
    gpio_set_level(OLED_RST_GPIO, 0);
    oled_delay_ms(200);
    gpio_set_level(OLED_RST_GPIO, 1);
    oled_delay_ms(100);
    return ESP_OK;
}

esp_err_t oled_gpio_init()
{
    ESP_LOGI(TAG, "init gpio");
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = OLED_PIN_SEL;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    ESP_LOGI(TAG, "init hspi");
    spi_config_t spi_config;
    // Load default interface parameters
    // CS_EN:1, MISO_EN:1, MOSI_EN:1, BYTE_TX_ORDER:1, BYTE_TX_ORDER:1, BIT_RX_ORDER:0, BIT_TX_ORDER:0, CPHA:0, CPOL:0
    spi_config.interface.val = SPI_DEFAULT_INTERFACE;
    // Load default interrupt enable
    // TRANS_DONE: true, WRITE_STATUS: false, READ_STATUS: false, WRITE_BUFFER: false, READ_BUFFER: false
    spi_config.intr_enable.val = SPI_MASTER_DEFAULT_INTR_ENABLE;
    // Cancel hardware cs
    spi_config.interface.cs_en = 0;
    // MISO pin is used for DC
    spi_config.interface.miso_en = 0;
    // CPOL: 1, CPHA: 1
    spi_config.interface.cpol = 1;
    spi_config.interface.cpha = 1;
    // Set SPI to master mode
    // 8266 Only support half-duplex
    spi_config.mode = SPI_MASTER_MODE;
    // Set the SPI clock frequency division factor
    spi_config.clk_div = SPI_10MHz_DIV;
    // Register SPI event callback function
    spi_config.event_cb = spi_event_callback;
    spi_init(HSPI_HOST, &spi_config);

    ESP_LOGI(TAG, "\nOLED_DC_GPIO:%d  OLED_RST_GPIO:%d",OLED_DC_GPIO,OLED_RST_GPIO);

  oled_rst(); // Reset OLED
  return ESP_OK;
}

// esp_err_t oled_Fully_fill(uint8_t data)
// {
//     uint8_t x;
//     uint32_t buf[16];
//     spi_trans_t trans = {0};
//     trans.mosi = buf;
//     trans.bits.mosi = 64 * 8;

//     for (x = 0; x < 16; x++) {
//         buf[x] = data << 24 | data << 16 | data << 8 | data;
//     }

//     // SPI transfers 64 bytes at a time, transmits twice, increasing the screen refresh rate
//     for (x = 0; x < 8; x++) {
//         oled_set_pos(0, x);
//         oled_set_dc(1);
//         spi_trans(HSPI_HOST, trans);
//         spi_trans(HSPI_HOST, trans);
//     }

//     return ESP_OK;
// }

void IRAM_ATTR spi_event_callback(int event, void *arg)
{
    switch (event) {
        case SPI_INIT_EVENT: {

        }
        break;

        case SPI_TRANS_START_EVENT: {
            gpio_set_level(OLED_DC_GPIO, oled_dc_level);
        }
        break;

        case SPI_TRANS_DONE_EVENT: {

        }
        break;

        case SPI_DEINIT_EVENT: {
        }
        break;
    }
}
