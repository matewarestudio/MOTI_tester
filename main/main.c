#include "main.h"

#include <stdio.h>
#include <stdlib.h>

#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_log.h"
#include "esp_random.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "esp_err.h"

// esp_err_t init_signals(void);

#define UPDATE_INTERVAL 500 // Tiempo de encendido/apagado^
#define QTY_INTERVAL_TO_SECOND (1000 / UPDATE_INTERVAL)
#define MAX_COUNT_IN_SECONDS 20
#define MAX_COUNT (MAX_COUNT_IN_SECONDS * QTY_INTERVAL_TO_SECOND)
#define MS_OF_SENSOR_TIMER 20
#define SHIFT_BITS_FOR_PERIODE 2 // Recorta el valor del ADC shifteando bits (12 bits usa)
#define WIDTH_DEFAULT 300
#define PERIODE_DEFAULT 1000


static const char *TAG = "Main";
TimerHandle_t xTimers; // Handler de timer
uint16_t count = 0;
const uint8_t timerId = 1, timerId_S = 2; // Identificador de timer
volatile uint32_t width_sensor_1 = WIDTH_DEFAULT;
volatile uint32_t width_sensor_2 = WIDTH_DEFAULT;
volatile uint32_t period_sensor_1 = PERIODE_DEFAULT;
volatile uint32_t period_sensor_2 = PERIODE_DEFAULT;

int adc_sensor_1 = 0;
int adc_sensor_2 = 0;
int min_bw = 1;
uint32_t rdm = 0;
uint32_t toff = 0;


esp_err_t gpio_init(void);
esp_err_t set_timer(void);
void vTimerCallback(TimerHandle_t pxTimer);
void vTimerCallback_S(TimerHandle_t pxTimer);

void app_main(void)
{
    vTaskDelay(100);
    gpio_init();
    set_timer();
    ESP_ERROR_CHECK(adc_init());
    rdm = esp_random();
}

void vTimerCallback(
    TimerHandle_t pxTimer) // tarea que ejecuta cada vez que termina el timer
{

    adc_read(&adc_sensor_1, 0);
    adc_read(&adc_sensor_2, 1);

    width_sensor_1 = ((uint32_t)adc_sensor_1) >> SHIFT_BITS_FOR_PERIODE;
    width_sensor_2 = ((uint32_t)adc_sensor_2) >> SHIFT_BITS_FOR_PERIODE;

    if (width_sensor_1 < min_bw)
    {
        width_sensor_1 = 0;
        gpio_set_level(PIN_SEM_SENSOR_1_SIGNAL, 1);
    }
   if(period_sensor_1>width_sensor_1){
    toff=period_sensor_1-width_sensor_1;
    }else{toff=0;}

    ESP_LOGE(TAG, "cant/min_1=%.1f period_sensor_1: %lu width_sensor_1 : %lu TOFF %lu",(float)(60*(1/((float)period_sensor_1/1000))), period_sensor_1, width_sensor_1, toff);

    if (width_sensor_2 < min_bw)
    {
        width_sensor_2 = 0;
        gpio_set_level(PIN_SEM_SENSOR_2_SIGNAL, 1);
    }
    if(period_sensor_2>width_sensor_2){
    toff=period_sensor_2-width_sensor_2;
    }else{toff=0;}

    ESP_LOGE(TAG, "cant/min_2=%.1f period_sensor_2: %lu width_sensor_2 : %lu TOFF %lu",(float)(60*(1/((float)period_sensor_2/1000))), period_sensor_2, width_sensor_2, toff);


    if (count < MAX_COUNT)
    {
        count++;
    }
    else
    {
        count = 0;
        rdm = esp_random() & MASK_BYTE;
        if(rdm < width_sensor_1)
        {
            rdm = width_sensor_1;
        }

        period_sensor_1 = rdm;
        ESP_LOGW(TAG, "rdm width_sensor_1 %lu", rdm);


        rdm = esp_random() & MASK_BYTE;
        if(rdm < width_sensor_2)
        {
            rdm = width_sensor_2;
        }
  
        period_sensor_2 = rdm;
        ESP_LOGW(TAG, "rdm width_sensor_2 %lu", rdm);
    }

}

void vTimerCallback_S(
    TimerHandle_t pxTimer) // tarea que ejecuta cada vez que termina el timer
{

    static uint32_t counter_width_1 = 0;
    static uint32_t counter_width_2 = 0;
    static uint32_t counter_period_1 = 0;
    static uint32_t counter_period_2 = 0;

    if (counter_period_1 < period_sensor_1)
    {

        counter_period_1+=MS_OF_SENSOR_TIMER;

        if (counter_width_1 < width_sensor_1)
        {
            counter_width_1+=MS_OF_SENSOR_TIMER;
            gpio_set_level(PIN_SEM_SENSOR_1_SIGNAL, 0);
        }
        else
        {
            gpio_set_level(PIN_SEM_SENSOR_1_SIGNAL, 1);
        }
    }
    else
    {
        counter_period_1 = 0;
        counter_width_1 = 0;
        gpio_set_level(PIN_SEM_SENSOR_1_SIGNAL, 1);
    }

    if (counter_period_2 < period_sensor_2)
    {

        counter_period_2+=MS_OF_SENSOR_TIMER;

        if (counter_width_2 < width_sensor_2)
        {
            counter_width_2+=MS_OF_SENSOR_TIMER;
            gpio_set_level(PIN_SEM_SENSOR_2_SIGNAL, 0);
        }
        else
        {
            gpio_set_level(PIN_SEM_SENSOR_2_SIGNAL, 1);
        }
    }
    else
    {
        counter_period_2 = 0;
        counter_width_2 = 0;
        gpio_set_level(PIN_SEM_SENSOR_2_SIGNAL, 1);
    }
}


esp_err_t set_timer(void)
{
    ESP_LOGI(TAG, "Timer init  configuration");

    // creamos el timer
    xTimers = xTimerCreate(
        "Timer",                          // Just a text name, not used by the kernel.
        (pdMS_TO_TICKS(UPDATE_INTERVAL)), // The timer period in ticks.
        pdTRUE,                           // The timers will auto-reload themselves when they expire.
        (void *)&timerId,                 // Assign each timer a unique id equal to its array index.
        vTimerCallback                    // Each timer calls the same callback when it expires.
    );

    if (xTimers == NULL)
    {
        ESP_LOGE(TAG, "The timer was not created");
    }
    else
    {
        if (xTimerStart(xTimers, 0) != pdPASS)
        {
            ESP_LOGE(TAG, "The timer could not be set into the Active state");
        }
    }

    xTimers = xTimerCreate(
        "Timer_Sensor",                      // Just a text name, not used by the kernel.
        (pdMS_TO_TICKS(MS_OF_SENSOR_TIMER)), // The timer period in ticks.
        pdTRUE,                              // The timers will auto-reload themselves when they expire.
        (void *)&timerId_S,                  // Assign each timer a unique id equal to its array index.
        vTimerCallback_S                     // Each timer calls the same callback when it expires.
    );

    if (xTimers == NULL)
    {
        ESP_LOGE(TAG, "The timer was not created");
    }
    else
    {
        if (xTimerStart(xTimers, 0) != pdPASS)
        {
            ESP_LOGE(TAG, "The timer could not be set into the Active state");
        }
    }

    return ESP_OK;
}

esp_err_t gpio_init(void)
{
    gpio_reset_pin(PIN_SEM_SENSOR_1_SIGNAL);
    gpio_set_direction(PIN_SEM_SENSOR_1_SIGNAL, GPIO_MODE_OUTPUT);
    gpio_set_level(PIN_SEM_SENSOR_1_SIGNAL, 1);

    gpio_reset_pin(PIN_SEM_SENSOR_2_SIGNAL);
    gpio_set_direction(PIN_SEM_SENSOR_2_SIGNAL, GPIO_MODE_OUTPUT);
    gpio_set_level(PIN_SEM_SENSOR_2_SIGNAL, 1);

    return ESP_OK;
}

