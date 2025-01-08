#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_err.h"
#include "adc_port.h"

const static char *TAG = "adc_port.c";

#define ADC_BW ADC_BITWIDTH_DEFAULT
#define ADC_ATT ADC_ATTEN_DB_12

adc_oneshot_unit_handle_t adc1_handle;
adc_oneshot_unit_init_cfg_t init_config1 = {
    .unit_id = ADC_UNIT,
};

adc_oneshot_chan_cfg_t config = {
    .bitwidth = ADC_BW,
    .atten = ADC_ATT,
};

// adc_oneshot_unit_handle_t adc1_handle_2;
// adc_oneshot_unit_init_cfg_t init_config_2 = {
//     .unit_id = ADC_UNIT,
// };

// adc_oneshot_chan_cfg_t config_2 = {
//     .bitwidth = ADC_BW,
//     .atten = ADC_ATT,
// };

esp_err_t adc_init(void)
{

  ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

  ESP_ERROR_CHECK(
      adc_oneshot_config_channel(adc1_handle, ADC_CH, &config));

  // ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config_2, &adc1_handle_2));

  // ESP_ERROR_CHECK(
  //     adc_oneshot_config_channel(adc1_handle_2, ADC_CH_2, &config_2));

  return ESP_OK;
}

esp_err_t adc_read(int *adc_raw, int adc_ch)
{

  if (adc_ch == 0)
  {

    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CH, adc_raw));

    ESP_LOGI(TAG, "ADC%d Channel[%d] Raw Data: %d", ADC_UNIT + 1, ADC_CH, *adc_raw);
  }
  else if (adc_ch == 1)
  {

    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CH_2, adc_raw));

    ESP_LOGI(TAG, "ADC%d Channel[%d] Raw Data: %d", ADC_UNIT + 1, ADC_CH_2, *adc_raw);
  }

  return ESP_OK;
}
