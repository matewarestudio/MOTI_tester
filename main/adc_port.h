#ifndef ADC_PORT_H
#define ADC_PORT_H

#include "esp_err.h"

////// BEGIN - ADC configurations
// ADC port for alarm configuration
#define ADC_UNIT ADC_UNIT_1
#define ADC_CH ADC_CHANNEL_5
#define ADC_CH_2 ADC_CHANNEL_4
////// END -ADC configurations

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t adc_init(void);
esp_err_t adc_read(int *adc_raw, int adc_ch);

#ifdef __cplusplus
}
#endif

#endif // ADC_PORT_H