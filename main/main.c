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


static const char *TAG = "Main";
TimerHandle_t xTimers;    // Handler de timer
uint16_t interval = 1000; // Tiempo de encendido/apagado^
const uint16_t max_count = 10;
uint16_t count = 0;
const uint8_t timerId = 1; // Identificador de timer
volatile uint32_t duty_PIN_SEM_SENSOR_1 = 0;
volatile uint32_t duty_PIN_SEM_SENSOR_2 = 0;
int ADC_PIN_SEM_SENSOR_1 = 0;
int ADC_PIN_SEM_SENSOR_2 = 0;
int MIN_BW = 1;
uint32_t rdm = 0;
ledc_timer_config_t timerConfig_PIN_SEM_SENSOR_1 = {0};
ledc_timer_config_t timerConfig_PIN_SEM_SENSOR_2 = {0};
bool calibration_flag;

// esp_err_t Define las funciones y permite retornar valores del estado del
// funcionamiento de la ejecución de la funcion esp_err_t init_signals(void); //
// defino la inicializacion
esp_err_t set_timer(void);    // configura timer
esp_err_t set_pwm(void);      // seteamos el pwm para cada led
esp_err_t set_pwm_duty(void); // seteamos el duty para cada pwm

void vTimerCallback(
    TimerHandle_t pxTimer) // tarea que ejecuta cada vez que termina el timer
{

adc_read(&ADC_PIN_SEM_SENSOR_1, 0);
adc_read(&ADC_PIN_SEM_SENSOR_2, 1);

    
    if (count < max_count)
    {
        count++;
    }
    else
    {
        count = 0;
        rdm = esp_random() & MASK_BYTE;
        ESP_LOGI(TAG, "%lu", rdm);
    }

        duty_PIN_SEM_SENSOR_1 = ((uint32_t) ADC_PIN_SEM_SENSOR_1) >> 2;
    if (rdm & (1 << BIT_MASK_SENSOR_1)) {
        // ESP_LOGE(TAG, "0 - duty_PIN_SEM_SENSOR_1 : %d", duty_PIN_SEM_SENSOR_1);
        timerConfig_PIN_SEM_SENSOR_1.freq_hz = 1;
        // duty_PIN_SEM_SENSOR_1*=2;
        ledc_timer_config(&timerConfig_PIN_SEM_SENSOR_1);
    } else {
        timerConfig_PIN_SEM_SENSOR_1.freq_hz = 2;
        // duty_PIN_SEM_SENSOR_1/=2;
        ledc_timer_config(&timerConfig_PIN_SEM_SENSOR_1);
    }
        if (duty_PIN_SEM_SENSOR_1 < MIN_BW) {
            duty_PIN_SEM_SENSOR_1 = 0;
            gpio_set_level(PIN_SEM_SENSOR_1_SIGNAL, 0);
        }

    ESP_LOGE(TAG, "f = %lu - duty_PIN_SEM_SENSOR_1 : %lu",timerConfig_PIN_SEM_SENSOR_1.freq_hz, duty_PIN_SEM_SENSOR_1);

        duty_PIN_SEM_SENSOR_2 = ((uint32_t) ADC_PIN_SEM_SENSOR_2) >> 2;
    if (rdm & (1 << BIT_MASK_SENSOR_2)) {
        // ESP_LOGE(TAG, "0 - duty_PIN_SEM_SENSOR_2 : %d", duty_PIN_SEM_SENSOR_2);
        timerConfig_PIN_SEM_SENSOR_2.freq_hz = 1;
        // duty_PIN_SEM_SENSOR_2*=2;
        ledc_timer_config(&timerConfig_PIN_SEM_SENSOR_2);
    } else {
        timerConfig_PIN_SEM_SENSOR_2.freq_hz = 2;
        // duty_PIN_SEM_SENSOR_2/=2;
        ledc_timer_config(&timerConfig_PIN_SEM_SENSOR_2);
    }
        if (duty_PIN_SEM_SENSOR_2 < MIN_BW) {
            duty_PIN_SEM_SENSOR_2 = 0;
            gpio_set_level(PIN_SEM_SENSOR_2_SIGNAL, 0);
        }

    ESP_LOGE(TAG, "f = %lu - duty_PIN_SEM_SENSOR_2 : %lu", timerConfig_PIN_SEM_SENSOR_2.freq_hz, duty_PIN_SEM_SENSOR_2);
   
    set_pwm_duty();
}

void app_main(void)
{
    vTaskDelay(100);
    // init_signals();
    set_pwm();
    set_timer();
    ESP_ERROR_CHECK(adc_init());
    rdm = esp_random();
}

esp_err_t set_timer(void)
{
    ESP_LOGI(TAG, "Timer init  configuration");

    // creamos el timer
    xTimers = xTimerCreate(
        "Timer",                   // Just a text name, not used by the kernel.
        (pdMS_TO_TICKS(interval)), // The timer period in ticks.
        pdTRUE,                    // The timers will auto-reload themselves when they expire.
        (void *)&timerId,          // Assign each timer a unique id equal to its array index.
        vTimerCallback             // Each timer calls the same callback when it expires.
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

esp_err_t set_pwm(void)
{

    ledc_channel_config_t channelConfig_PIN_SEM_SENSOR_1 = {0};
    channelConfig_PIN_SEM_SENSOR_1.gpio_num = PIN_SEM_SENSOR_1_SIGNAL;
    channelConfig_PIN_SEM_SENSOR_1.speed_mode = LEDC_LOW_SPEED_MODE;
    channelConfig_PIN_SEM_SENSOR_1.channel = LEDC_CHANNEL_1;
    channelConfig_PIN_SEM_SENSOR_1.intr_type = LEDC_INTR_DISABLE;
    channelConfig_PIN_SEM_SENSOR_1.timer_sel = LEDC_TIMER_1;
    channelConfig_PIN_SEM_SENSOR_1.duty = 0;

    ledc_channel_config_t channelConfig_PIN_SEM_SENSOR_2 = {0};
    channelConfig_PIN_SEM_SENSOR_2.gpio_num = PIN_SEM_SENSOR_2_SIGNAL;
    channelConfig_PIN_SEM_SENSOR_2.speed_mode = LEDC_LOW_SPEED_MODE;
    channelConfig_PIN_SEM_SENSOR_2.channel = LEDC_CHANNEL_2;
    channelConfig_PIN_SEM_SENSOR_2.intr_type = LEDC_INTR_DISABLE;
    channelConfig_PIN_SEM_SENSOR_2.timer_sel = LEDC_TIMER_2;
    channelConfig_PIN_SEM_SENSOR_2.duty = 0;

    ledc_channel_config(&channelConfig_PIN_SEM_SENSOR_1);
    ledc_channel_config(&channelConfig_PIN_SEM_SENSOR_2);

    timerConfig_PIN_SEM_SENSOR_1.speed_mode = LEDC_LOW_SPEED_MODE;
    timerConfig_PIN_SEM_SENSOR_1.duty_resolution = LEDC_TIMER_10_BIT;
    timerConfig_PIN_SEM_SENSOR_1.timer_num = LEDC_TIMER_1;
    timerConfig_PIN_SEM_SENSOR_1.clk_cfg = LEDC_AUTO_CLK;
    timerConfig_PIN_SEM_SENSOR_1.freq_hz = 1;
 
    timerConfig_PIN_SEM_SENSOR_2.speed_mode = LEDC_LOW_SPEED_MODE;
    timerConfig_PIN_SEM_SENSOR_2.duty_resolution = LEDC_TIMER_10_BIT;
    timerConfig_PIN_SEM_SENSOR_2.timer_num = LEDC_TIMER_2;
    timerConfig_PIN_SEM_SENSOR_2.clk_cfg = LEDC_AUTO_CLK;
    timerConfig_PIN_SEM_SENSOR_2.freq_hz = 1;

    ledc_timer_config(&timerConfig_PIN_SEM_SENSOR_1);
    ledc_timer_config(&timerConfig_PIN_SEM_SENSOR_2);

    return ESP_OK;
}

esp_err_t set_pwm_duty(void)
{
    // Setea el duty
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, duty_PIN_SEM_SENSOR_1);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2, duty_PIN_SEM_SENSOR_2);
    // Actualiza el duty
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2);

    return ESP_OK;
}


