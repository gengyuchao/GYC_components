
/*
  This example code is created by gengyuchao used for Latest ESP8266 
  SDK based on FreeRTOS, esp-idf style.  (update:2019-07-24).
  Unless required by applicable law or agreed to in writing, this
  software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
  CONDITIONS OF ANY KIND, either express or implied.
 */

#ifndef __DHT11_DRIVER__
#define __DHT11_DRIVER__

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp8266/gpio_struct.h"

#include "esp_system.h"
#include "esp_log.h"
#include "esp_libc.h"
#include "driver/gpio.h"

#include "string.h"
#include "unistd.h"


typedef enum {
	DHT11,
	DHT22
} DHTType;

typedef struct {
  float temperature;
  float humidity;
} DHT_Sensor_Data;

#define DHT_MAXTIMINGS	10000
#define DHT_BREAKTIME	20
#define DHT_MAXCOUNT	32000

class DHT_Sensor
{
  public:
      uint8_t data_pin;
      DHTType sensor_type;

      DHT_Sensor(uint8_t pin, DHTType type);

      DHT_Sensor_Data Sensor_Data;
      bool DHTInit();
      bool DHTRead(); 
      bool DHTRead(DHT_Sensor_Data* output);     
  private:
     float scale_humidity(int *data);
     float scale_temperature(int *data);
};





#endif
