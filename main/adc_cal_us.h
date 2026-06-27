#ifndef ADC_CAL_US_H_
#define ADC_CAL_US_H_

#include<stdint.h>
#include "driver/gptimer.h"
#include "hal/adc_types.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

extern adc_oneshot_unit_handle_t adc1_handle;
extern adc_cali_handle_t cali_handle;
void adc_init(adc_oneshot_unit_handle_t *adc,adc_cali_handle_t *cali);
#endif