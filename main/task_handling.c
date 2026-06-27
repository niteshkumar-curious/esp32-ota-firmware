#include "task_handling.h"
#include "common_header.h"
#include "mydata.h"
#include "mqtt_ota_connect.h"
#include "gpio_user_config.h"
#include "adc_cal_us.h"
#include <math.h>

volatile float latest_temp=0.0f;

/* ================= OTA LED TASK ================= */

static void ota_led_task(void *arg)
{
    while (1)
    {
        if (ota_running)
        {
            gpio_set_level(OTA_LED_GPIO, 1);

            vTaskDelay(pdMS_TO_TICKS(500));

            gpio_set_level(OTA_LED_GPIO, 0);

            vTaskDelay(pdMS_TO_TICKS(500));
        }
        else
        {
            gpio_set_level(OTA_LED_GPIO, 0);

            vTaskDelay(pdMS_TO_TICKS(500));
        }
    }
}

/* ================= STATUS HEARTBEAT ================= */

static void status_task(void *arg)
{
    char temp_str[16];

    while (1)
    {

        if (mqtt_connected)
        {
            esp_mqtt_client_publish(
                mqtt_get_client(),
                MQTT_TOPIC_STATUS,
                "online",
                0,
                1,
                1);

            esp_mqtt_client_publish(
                mqtt_get_client(),
                MQTT_TOPIC_VERSION,
                FW_VERSION,
                0,
                1,
                0);
            sprintf(temp_str,"%.2f",latest_temp);
            esp_mqtt_client_publish(
                mqtt_get_client(),
                MQTT_TOPIC_TEMPERATURE,
                temp_str,
                0,
                1,
                0
            );
        }
        

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/* ================= Sensor TASK ================= */

static void sensor_task(void *arg)
{
    static const float R_fixed = 9800.0f;
    static const float T0 = 298.15f;  // 25 * C in kelvin
    static const float R0 = 10000.0f; // 10k at 25C
    static const float B = 3950.0f;   // 3435 3470 3950 3977 3380
    int data=0;
    long long sum_data=0;
    uint32_t duty;

    while (1)
    {
        sum_data = 0;

        for (int i = 0; i < 5; i++)
        {
            adc_oneshot_get_calibrated_result(
                adc1_handle,
                cali_handle,
                ADC_CHANNEL_6,
                &data);

            sum_data += data;
        }

        int avg_sum = sum_data / 5;

        float voltage = avg_sum / 1000.0f;

        float R_ntc = (voltage * R_fixed) / (3.33f - voltage);

        float tempk = 1.0f /
                      ((1.0f / T0) +
                       (log(R_ntc / R0) / B));

        float tempC = tempk - 273.15f;

        duty = (avg_sum * 8191) / 3330;

    
        pwm_set_duty(duty);
    
        latest_temp = tempC;

        /*printf("ADC=%d Temp=%.2f PWM=%lu\n",
               avg_sum,
               tempC,
               duty);*/


        vTaskDelay(pdMS_TO_TICKS(100));
    }
}



void create_tasks(void)
{

    xTaskCreate(sensor_task, "sensor_task", 4096, NULL, 5, NULL);

    xTaskCreate(ota_led_task, "ota_led", 2048, NULL, 4, NULL);

    xTaskCreate(status_task, "status_task", 4096, NULL, 5, NULL);
}