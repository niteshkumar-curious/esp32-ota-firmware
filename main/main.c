#include "common_header.h"
#include "mydata.h"

#include "gpio_user_config.h"
#include "wifi.h"
#include "mqtt_ota_connect.h"
#include "task_handling.h"
#include "adc_cal_us.h"

adc_oneshot_unit_handle_t adc1_handle;
adc_cali_handle_t cali_handle;

void app_main(void)
{
    esp_err_t ret =
        nvs_flash_init();

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(
            nvs_flash_erase());

        ret = nvs_flash_init();
    }

    ESP_ERROR_CHECK(ret);

    /* Hardware Init */

    
    gpio_init();
    
    adc_init(&adc1_handle, &cali_handle);
    pwm_init();


    /* Network Init */

    wifi_init();

    obtain_time();

    mqtt_app_start();

    /* Allow MQTT connection */

    vTaskDelay(pdMS_TO_TICKS(3000));

    /* FreeRTOS Tasks */

    create_tasks();

    /* OTA Rollback Validation */

    validate_new_firmware();
}
