#include "mqtt_ota_connect.h"
#include "common_header.h"
#include "mydata.h"
#include "wifi.h"

static const char *TAG = "MQTT_LED";
static esp_mqtt_client_handle_t mqtt_client = NULL;
bool mqtt_connected = false;
bool ota_running = false;
char ota_url[300] = {0};
char target_version[32] = {0};

/* ================= MQTT EVENT ================= */

static void mqtt_event_handler(
    void *handler_args,
    esp_event_base_t base,
    int32_t event_id,
    void *event_data)
{
    esp_mqtt_event_handle_t event =
        event_data;

    switch ((esp_mqtt_event_id_t)event_id)
    {

    case MQTT_EVENT_CONNECTED:

        ESP_LOGI(TAG, "MQTT connected");

        mqtt_connected = true;

        esp_mqtt_client_subscribe(
            mqtt_client,
            MQTT_TOPIC_OTA,
            1);

        esp_mqtt_client_publish(
            mqtt_client,
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


        if (strncmp(
                event->topic,
                MQTT_TOPIC_OTA,
                event->topic_len) == 0)
        {
            char data[512] = {0};

            memcpy(
                data,
                event->data,
                event->data_len);

            cJSON *root =
                cJSON_Parse(data);

            if (root)
            {
                cJSON *url =
                    cJSON_GetObjectItem(
                        root,
                        "url");

                cJSON *version =
                    cJSON_GetObjectItem(
                        root,
                        "version");

                if (url && version)
                {
                    strcpy(
                        ota_url,
                        url->valuestring);

                    strcpy(
                        target_version,
                        version->valuestring);

                    ESP_LOGI(
                        TAG,
                        "OTA Version: %s",
                        target_version);

                    ESP_LOGI(
                        TAG,
                        "OTA URL: %s",
                        ota_url);

                    start_ota_update();
                }

                cJSON_Delete(root);
            }
        }

        break;

    default:
        break;
    }
}

/* ================= MQTT INIT ================= */

void mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {

        .broker.address.uri = MQTT_URI,

        .credentials.username =
            MQTT_USER,

        .credentials.authentication.password =
            MQTT_PASS,

        .broker.verification.crt_bundle_attach =
            esp_crt_bundle_attach,

        .session.last_will.topic =
            MQTT_TOPIC_STATUS,

        .session.last_will.msg =
            "offline",

        .session.last_will.qos = 1,

        .session.last_will.retain = 1,
    };

    mqtt_client =
        esp_mqtt_client_init(
            &mqtt_cfg);

    esp_mqtt_client_register_event(
        mqtt_client,
        ESP_EVENT_ANY_ID,
        mqtt_event_handler,
        NULL);

    esp_mqtt_client_start(
        mqtt_client);
}

esp_mqtt_client_handle_t mqtt_get_client(void)
{
    return mqtt_client;
}

/* ================= OTA UPDATE ================= */

void start_ota_update()
{
    ESP_LOGI(TAG, "Starting OTA update");

    ota_running = true;

    esp_http_client_config_t http_config = {
        .url = ota_url,
        .crt_bundle_attach = esp_crt_bundle_attach,
        .timeout_ms = 5000,
    };

    esp_https_ota_config_t ota_config = {
        .http_config = &http_config,
    };

    esp_https_ota_handle_t https_ota_handle = NULL;

    if (esp_https_ota_begin(
            &ota_config,
            &https_ota_handle) != ESP_OK)
    {
        ESP_LOGE(TAG, "OTA begin failed");

        ota_running = false;

        return;
    }

    int last_percent = 0;

    while (1)
    {
        esp_err_t err =
            esp_https_ota_perform(
                https_ota_handle);

        if (err != ESP_ERR_HTTPS_OTA_IN_PROGRESS)
            break;

        int downloaded =
            esp_https_ota_get_image_len_read(
                https_ota_handle);

        int total =
            esp_https_ota_get_image_size(
                https_ota_handle);

        if (total > 0)
        {
            int percent =
                (downloaded * 100) / total;

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

    if (esp_https_ota_finish(
            https_ota_handle) == ESP_OK)
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

/* ================= OTA VALIDATION ================= */

void validate_new_firmware(void)
{
    const esp_partition_t *running =
        esp_ota_get_running_partition();

    esp_ota_img_states_t ota_state;

    if (esp_ota_get_state_partition(
            running,
            &ota_state) == ESP_OK)
    {
        if (ota_state ==
            ESP_OTA_IMG_PENDING_VERIFY)
        {
            ESP_LOGI(
                TAG,
                "New firmware pending verification");

            bool self_test_passed = true;

            /* ---------- HEALTH CHECKS ---------- */

           if (!(wifi_is_connected()))
            {
                ESP_LOGE(
                    TAG,
                    "WiFi failed");

                self_test_passed = false;
            }


            /* ---------- RESULT ---------- */

            if (self_test_passed)
            {
                ESP_LOGI(
                    TAG,
                    "Firmware valid");

                esp_ota_mark_app_valid_cancel_rollback();
            }
            else
            {
                ESP_LOGE(
                    TAG,
                    "Firmware invalid");

                esp_ota_mark_app_invalid_rollback_and_reboot();
            }
        }
    }
}
