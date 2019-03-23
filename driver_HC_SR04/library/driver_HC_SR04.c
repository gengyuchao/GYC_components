/*******************************************************************************

    Driver for HC-SR04 

  引脚连接：Trig--D6   Echo--D5
*******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "driver/gpio.h"
#include "driver/hw_timer.h"

#include "esp_log.h"
#include "esp_system.h"

#include "socket_pack.h"
#include "driver_HC_SR04.h"
#include "rom/gpio.h"

static const char *TAG = "ultrasonic";
float Distance=0;
static unsigned int us_count=0;

#define TIMER_CLK_DIV TIMER_CLKDIV_16 //分频系数

#define WAIT_WAVE_BACK_TIME (0x3FFFFF>>TIMER_CLK_DIV)  //返回波形最大等待时间

#define Trig_GPIO_PIN 12    //Trig D6

#define Echo_GPIO_PIN 14    //Echo D5

int32_t clock_count= WAIT_WAVE_BACK_TIME;


#define GPIO_OUTPUT_PIN_SEL  (1ULL<<Trig_GPIO_PIN) 

#define GPIO_INPUT_PIN_SEL  ((1ULL<<Echo_GPIO_PIN))


static void gpio_isr_handler(void *arg)
{
    uint32_t gpio_num = (uint32_t) arg;

    if (GPIO_INPUT_GET(Echo_GPIO_PIN) ==1) {
            clock_count = WAIT_WAVE_BACK_TIME;
            hw_timer_set_load_data((uint32_t)clock_count);
            hw_timer_enable(true);
    }
    
    if (GPIO_INPUT_GET(Echo_GPIO_PIN)==0) { //①引脚中断模式

            // while(GPIO_INPUT_GET(Echo_GPIO_PIN)==1); //②一直等待模式

            clock_count=WAIT_WAVE_BACK_TIME - hw_timer_get_count_data();
            hw_timer_enable(false);

            if(clock_count>0)
            {
                us_count=clock_count/((800>> hw_timer_get_clkdiv())/10.0);// load/((TIMER_BASE_CLK >> hw_timer_get_clkdiv()) / 1000000) =( us);
                
                Distance=us_count*340.0f/20000.0f;

                // ets_printf("clock_count:%d\n",clock_count);
                // ets_printf("us_count:%d\n",us_count);
                // ets_printf("Distance:%f\n",Distance);                
            }
            else
            {
                us_count=0;
                // ets_printf("Ultrasonic wait out of range\n"); 
            }
    }
}

static void hw_timer_us_count()
{
    ;
}

esp_err_t hw_timer_count_us()
{
    uint32_t reload_value = WAIT_WAVE_BACK_TIME;
    
    hw_timer_set_reload(false);
    hw_timer_set_clkdiv(TIMER_CLK_DIV);
    hw_timer_set_intr_type(TIMER_EDGE_INT);
    hw_timer_set_load_data(reload_value);   // Calculate the number of timer clocks required for timing, ticks = (80MHz / div) * t
    hw_timer_enable(false);

    return ESP_OK;
}

void HC_SR04_Init()
{
    gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO15/16
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

    //interrupt of rising edge
    io_conf.intr_type = GPIO_INTR_ANYEDGE;
    //bit mask of the pins, use GPIO4/5 here
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    //set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    //enable pull-up mode
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    //install gpio isr service
    gpio_install_isr_service(0);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(Echo_GPIO_PIN, gpio_isr_handler, (void *) Echo_GPIO_PIN);

    ESP_LOGI(TAG, "Initialize hw_timer for callback ");
    hw_timer_init(hw_timer_us_count, NULL);
    ESP_LOGI(TAG, "Set hw_timer timing count with reload");
    hw_timer_count_us();
}

void HC_SR04_Send_Wave()
{
    gpio_set_level(Trig_GPIO_PIN, 1);

    os_delay_us(20);

    gpio_set_level(Trig_GPIO_PIN, 0);
}

void task_Get_Distance(void* time_xms)
{
    int count = 0;
    HC_SR04_Init();
    uint32_t period =*(uint32_t*)time_xms;

    if(period<60)
    {
        ets_printf("Ultrasonic's period is out of Range! It mast more than 60! currect value:%d\n",period);
        period=60;    
    }
    else
    {
        ets_printf("Ultrasonic's period OK! currect period value:%d\n",period);
    }

    while(1)
    {
        HC_SR04_Send_Wave();

        // ets_printf("clock_count:%d\n",clock_count);
        // ets_printf("us_count:%d\n",us_count);
        
        if(us_count!=0)
        {
            tcp_printf("count :%d ,Distance:%.2f\n",count++,Distance); 
        }
        else
        {
            tcp_printf("Ultrasonic Wait Out of Time.\n"); 
        }

        vTaskDelay(period / portTICK_RATE_MS);
    }
}

