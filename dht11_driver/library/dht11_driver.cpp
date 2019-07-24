
/*
  This example code is created by gengyuchao used for Latest ESP8266 
  SDK based on FreeRTOS, esp-idf style.  (update:2019-07-24).
  Unless required by applicable law or agreed to in writing, this
  software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
  CONDITIONS OF ANY KIND, either express or implied.
 */


#include "dht11_driver.hpp"


#define delay_us usleep
#define sleepms(x) vTaskDelay(x / portTICK_RATE_MS);//os_delay_us(x*1000);


//调试步骤打印开关 屏蔽定义 DHT_DEBUG 可以关闭调试信息在控制台的输出
// #define DHT_DEBUG

#ifdef DHT_DEBUG
#undef DHT_DEBUG
#define DHT_DEBUG(...) os_printf(__VA_ARGS__);
#else
#define DHT_DEBUG(...)
#endif


//从原始数据获取湿度信息
float DHT_Sensor::scale_humidity( int *data)
{
	if(sensor_type == DHT11) {
		return (float) data[0];
	} else {
		float humidity = data[0] * 256 + data[1];
		return humidity /= 10;
	}
}

//从原始属于获取湿度信息
float DHT_Sensor::scale_temperature( int *data)
{
	if(sensor_type == DHT11) {
		return (float) data[2];
	} else {
		float temperature = data[2] & 0x7f;
		temperature *= 256;
		temperature += data[3];
		temperature /= 10;
		if (data[2] & 0x80)
			temperature *= -1;
		return temperature;
	}
}


//设置温湿度传感器的引脚和型号(DHT11 or DHT22) 
DHT_Sensor::DHT_Sensor(uint8_t pin, DHTType type)
{
	data_pin=pin;
	sensor_type=type;
	DHTInit();
}

//温湿度传感器硬件初始化
bool DHT_Sensor::DHTInit()
{
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL<<(data_pin));
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;

	if (gpio_config(&io_conf)==ESP_OK) {
		DHT_DEBUG("DHT: Setup for type %s connected to GPIO%d\n", sensor_type==DHT11?"DHT11":"DHT22", data_pin);
		return true;
	} else {
		DHT_DEBUG("DHT: Error in function set_gpio_mode for type %s connected to GPIO%d\n", sensor_type==DHT11?"DHT11":"DHT22", data_pin);
		return false;
	}
}

//温湿度传感器数据读取
bool DHT_Sensor::DHTRead()
{
	DHT_Sensor_Data* output=&Sensor_Data;
	return DHTRead(output);
}

//温湿度传感器数据读取（并将数据写入到传入指针）
bool DHT_Sensor::DHTRead(DHT_Sensor_Data* output)
{
	int counter = 0;
	int laststate = 1;
	int i = 0;
	int j = 0;
	int checksum = 0;
	int data[100];
	data[0] = data[1] = data[2] = data[3] = data[4] = 0;
	gpio_num_t pin = (gpio_num_t)data_pin;

	gpio_set_direction(pin, GPIO_MODE_OUTPUT);
	// Wake up device, 250ms of high
	gpio_set_level(pin, 1);
	sleepms(250);
	// Hold low for 20ms
	gpio_set_level(pin, 0);
	sleepms(20);
	// High for 40ns
	gpio_set_level(pin, 1);
	delay_us(40);
	// Set DHT_PIN pin as an input
	gpio_set_direction(pin, GPIO_MODE_INPUT); //GPIO_DIS_OUTPUT(pin);

	// wait for pin to drop?
	while (gpio_get_level(pin) == 1 && i < DHT_MAXCOUNT) {
		delay_us(1);
		i++;
	}

	if(i == DHT_MAXCOUNT) {
		DHT_DEBUG("DHT: Failed to get reading from GPIO%d, dying\r\n", pin);
	    return false;
	}

	// read data
	for (i = 0; i < DHT_MAXTIMINGS; i++)
	{
		// Count high time (in approx us)
		counter = 0;
		while (gpio_get_level(pin) == laststate) {
			counter++;
			delay_us(1);
			if (counter == 1000)
				break;
		}
		laststate = gpio_get_level(pin);
		if (counter == 1000)
			break;
		// store data after 3 reads
		if ((i>3) && (i%2 == 0)) {
			// shove each bit into the storage bytes
			data[j/8] <<= 1;
			if (counter > DHT_BREAKTIME)
				data[j/8] |= 1;
			j++;
		}
	}

	if (j >= 39) {
		checksum = (data[0] + data[1] + data[2] + data[3]) & 0xFF;
	    DHT_DEBUG("DHT%s: %02x %02x %02x %02x [%02x] CS: %02x (GPIO%d)\r\n",
	              sensor_type==DHT11?"11":"22",
	              data[0], data[1], data[2], data[3], data[4], checksum, pin);
		if (data[4] == checksum) {
			// checksum is valid
			output->temperature = scale_temperature(data);
			output->humidity = scale_humidity(data);
			DHT_DEBUG("DHT: Temperature*100 =  %d *C, Humidity*100 = %d %% (GPIO%d)\n",
		          (int) (output->temperature * 100), (int) (output->humidity * 100), pin);
		} else {
			DHT_DEBUG("DHT: Checksum was incorrect after %d bits. Expected %d but got %d (GPIO%d)\r\n",
		                j, data[4], checksum, pin);
		    return false;
		}
	} else {
	    DHT_DEBUG("DHT: Got too few bits: %d should be at least 40 (GPIO%d)\r\n", j, pin);
	    return false;
	}
	return true;
}





