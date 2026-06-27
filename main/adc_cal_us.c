#include "adc_cal_us.h"


void adc_init(adc_oneshot_unit_handle_t *adc,adc_cali_handle_t *cali)
{
    
    adc_oneshot_unit_init_cfg_t init_congig1 = {
        .unit_id = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };

    adc_oneshot_new_unit(&init_congig1, &adc1_handle);
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_12,
        .atten = ADC_ATTEN_DB_12};

    adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_6, &config);

    adc_cali_line_fitting_config_t calfit = {
        .unit_id = ADC_UNIT_1,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_12
    };
    
    adc_cali_create_scheme_line_fitting(&calfit, &cali_handle);
}

