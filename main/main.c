#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "mydata.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"

#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "nvs_flash.h"

#include "esp_sntp.h"
#include "driver/gpio.h"

#include "mqtt_client.h"
#include "esp_crt_bundle.h"

#include "esp_https_ota.h"
#include "esp_ota_ops.h"

/* ================= USER CONFIG ================= */

#define MQTT_TOPIC_CMD "esp32/device1/led"
#define MQTT_TOPIC_STATUS "esp32/device1/status"
#define MQTT_TOPIC_OTA "esp32/device1/ota"
#define MQTT_TOPIC_OTA_PROGRESS "esp32/device1/ota_progress"
#define MQTT_TOPIC_OTA_RESULT "esp32/device1/ota_result"

#define LED_GPIO GPIO_NUM_2
#define BUTTON_GPIO GPIO_NUM_4
#define OTA_LED_GPIO GPIO_NUM_5

#define OTA_FIRMWARE_URL "https://github.com/niteshkumar-curious/esp32-ota-firmware/raw/refs/heads/main/firmware_ota/esp32_upt.bin"

/* ================= GLOBALS ================= */

static const char *TAG = "MQTT_LED";

static EventGroupHandle_t wifi_event_group;
#define WIFI_CONNECTED_BIT BIT0

static esp_mqtt_client_handle_t mqtt_client = NULL;
static QueueHandle_t gpio_evt_queue = NULL;

static int led_state = 0;
static bool ota_running = false;
static bool mqtt_connected = false;

/* ================= OTA LED TASK ================= */

static void ota_led_task(void *arg)
{
    while (1)
    {
        if (ota_running)
        {
            gpio_set_level(OTA_LED_GPIO, 1);
            vTaskDelay(pdMS_TO_TICKS(800));
            gpio_set_level(OTA_LED_GPIO, 0);
            vTaskDelay(pdMS_TO_TICKS(800));
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
    while (1)
    {
        if (mqtt_connected)
        {
            esp_mqtt_client_publish(
                mqtt_client,
                MQTT_TOPIC_STATUS,
                "online",
                0,
                1,
                1);
        }

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

/* ================= OTA UPDATE ================= */

static void start_ota_update()
{
    ESP_LOGI(TAG, "Starting OTA update");

    ota_running = true;

    esp_http_client_config_t http_config = {
        .url = OTA_FIRMWARE_URL,
        .crt_bundle_attach = esp_crt_bundle_attach,
        .timeout_ms = 5000,
    };

    esp_https_ota_config_t ota_config = {
        .http_config = &http_config,
    };

    esp_https_ota_handle_t https_ota_handle = NULL;

    if (esp_https_ota_begin(&ota_config, &https_ota_handle) != ESP_OK)
    {
        ESP_LOGE(TAG, "OTA begin failed");
        ota_running = false;
        return;
    }

    int last_percent = 0;

    while (1)
    {
        esp_err_t err = esp_https_ota_perform(https_ota_handle);

        if (err != ESP_ERR_HTTPS_OTA_IN_PROGRESS)
            break;

        int downloaded = esp_https_ota_get_image_len_read(https_ota_handle);
        int total = esp_https_ota_get_image_size(https_ota_handle);

        if (total > 0)
        {
            int percent = (downloaded * 100) / total;

            if (percent != last_percent)
            {
                char msg[10];
                sprintf(msg, "%d", percent);

                esp_mqtt_client_publish(
                    mqtt_client,
                    MQTT_TOPIC_OTA_PROGRESS,
                    msg,
                    0,
                    1,
                    0);

                last_percent = percent;
            }
        }
    }

    if (esp_https_ota_finish(https_ota_handle) == ESP_OK)
    {
        ESP_LOGI(TAG, "OTA successful");

        esp_mqtt_client_publish(
            mqtt_client,
            MQTT_TOPIC_OTA_PROGRESS,
            "100",
            0,
            1,
            0);

        esp_mqtt_client_publish(
            mqtt_client,
            MQTT_TOPIC_OTA_RESULT,
            "success",
            0,
            1,
            0);

        gpio_set_level(OTA_LED_GPIO, 1);
        vTaskDelay(pdMS_TO_TICKS(3000));

        esp_restart();
    }
    else
    {
        ESP_LOGE(TAG, "OTA failed");

        esp_mqtt_client_publish(
            mqtt_client,
            MQTT_TOPIC_OTA_RESULT,
            "failed",
            0,
            1,
            0);

        ota_running = false;
    }
}

/* ================= WIFI ================= */

static void wifi_event_handler(void *arg,
                               esp_event_base_t event_base,
                               int32_t event_id,
                               void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
        esp_wifi_connect();

    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        ESP_LOGI(TAG, "WiFi reconnecting...");
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }

    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ESP_LOGI(TAG, "WiFi connected");
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

static void wifi_init(void)
{
    wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    xEventGroupWaitBits(wifi_event_group,
                        WIFI_CONNECTED_BIT,
                        pdFALSE,
                        pdFALSE,
                        portMAX_DELAY);
}

/* ================= TIME SYNC ================= */

static void obtain_time(void)
{
    esp_sntp_stop();
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "time.google.com");
    esp_sntp_init();

    time_t now = 0;
    struct tm timeinfo = {0};

    while (timeinfo.tm_year < (2023 - 1900))
    {
        vTaskDelay(pdMS_TO_TICKS(2000));
        time(&now);
        localtime_r(&now, &timeinfo);
    }
}

/* ================= BUTTON ISR ================= */

static void IRAM_ATTR button_isr_handler(void *arg)
{
    uint32_t gpio_num = (uint32_t)arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

/* ================= BUTTON TASK ================= */

static void button_task(void *arg)
{
    uint32_t io_num;

    while (1)
    {
        if (xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY))
        {
            vTaskDelay(pdMS_TO_TICKS(150));

            if (gpio_get_level(BUTTON_GPIO) == 0)
            {
                led_state = !led_state;
                gpio_set_level(LED_GPIO, led_state);

                char msg = led_state ? '1' : '0';

                if (mqtt_connected)
                {
                    esp_mqtt_client_publish(
                        mqtt_client,
                        MQTT_TOPIC_CMD,
                        &msg,
                        1,
                        1,
                        1);
                }
            }
        }
    }
}

/* ================= MQTT EVENT ================= */

static void mqtt_event_handler(void *handler_args,
                               esp_event_base_t base,
                               int32_t event_id,
                               void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;

    switch ((esp_mqtt_event_id_t)event_id)
    {

    case MQTT_EVENT_CONNECTED:

        ESP_LOGI(TAG, "MQTT connected");
        mqtt_connected = true;

        esp_mqtt_client_subscribe(mqtt_client, MQTT_TOPIC_CMD, 1);
        esp_mqtt_client_subscribe(mqtt_client, MQTT_TOPIC_OTA, 1);

        esp_mqtt_client_publish(mqtt_client,
                                MQTT_TOPIC_STATUS,
                                "online",
                                0,
                                1,
                                1);
        break;

    case MQTT_EVENT_DISCONNECTED:

        ESP_LOGW(TAG, "MQTT disconnected");
        mqtt_connected = false;
        break;

    case MQTT_EVENT_DATA:

        if (strncmp(event->topic, MQTT_TOPIC_CMD, event->topic_len) == 0)
        {
            int value = (event->data[0] == '1') ? 1 : 0;
            led_state = value;
            gpio_set_level(LED_GPIO, led_state);
        }

        if (strncmp(event->topic, MQTT_TOPIC_OTA, event->topic_len) == 0)
        {
            ESP_LOGI(TAG, "OTA command received");
            start_ota_update();
        }

        break;

    default:
        break;
    }
}

/* ================= MQTT INIT ================= */

static void mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {

        .broker.address.uri = MQTT_URI,

        .credentials.username = MQTT_USER,
        .credentials.authentication.password = MQTT_PASS,

        .broker.verification.crt_bundle_attach = esp_crt_bundle_attach,

        .session.last_will.topic = MQTT_TOPIC_STATUS,
        .session.last_will.msg = "offline",
        .session.last_will.qos = 1,
        .session.last_will.retain = 1,
    };

    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);

    esp_mqtt_client_register_event(
        mqtt_client,
        ESP_EVENT_ANY_ID,
        mqtt_event_handler,
        NULL);

    esp_mqtt_client_start(mqtt_client);
}

/* ================= MAIN ================= */

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    ESP_ERROR_CHECK(ret);

    gpio_config_t led_conf = {
        .pin_bit_mask = (1ULL << LED_GPIO) | (1ULL << OTA_LED_GPIO),
        .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config(&led_conf);

    gpio_config_t btn_conf = {
        .pin_bit_mask = (1ULL << BUTTON_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .intr_type = GPIO_INTR_NEGEDGE,
    };
    gpio_config(&btn_conf);

    gpio_evt_queue = xQueueCreate(3, sizeof(uint32_t));

    gpio_install_isr_service(0);

    gpio_isr_handler_add(BUTTON_GPIO,
                         button_isr_handler,
                         (void *)BUTTON_GPIO);

    xTaskCreate(button_task, "button_task", 2048, NULL, 5, NULL);
    xTaskCreate(ota_led_task, "ota_led", 2048, NULL, 4, NULL);
    xTaskCreate(status_task, "status_task", 2048, NULL, 4, NULL);

    wifi_init();
    obtain_time();
    mqtt_app_start();
}