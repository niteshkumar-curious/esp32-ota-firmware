#include "wifi.h"
#include "common_header.h"
#include "mydata.h"

static EventGroupHandle_t wifi_event_group;
static const char *TAG = "WIFI";
#define WIFI_CONNECTED_BIT BIT0

/* ================= WIFI ================= */

static void wifi_event_handler(
    void *arg,
    esp_event_base_t event_base,
    int32_t event_id,
    void *event_data)
{
    if (event_base == WIFI_EVENT &&
        event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }

    else if (event_base == WIFI_EVENT &&
             event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        ESP_LOGI(TAG, "WiFi reconnecting...");

        esp_wifi_connect();

        xEventGroupClearBits(
            wifi_event_group,
            WIFI_CONNECTED_BIT);
    }

    else if (event_base == IP_EVENT &&
             event_id == IP_EVENT_STA_GOT_IP)
    {
        ESP_LOGI(TAG, "WiFi connected");

        xEventGroupSetBits(
            wifi_event_group,
            WIFI_CONNECTED_BIT);
    }
}

/*                     wifi init                         */

void wifi_init(void)
{
    wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(
        esp_event_loop_create_default());

    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg;
    cfg = (wifi_init_config_t)WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(
        esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(
        esp_event_handler_instance_register(
            WIFI_EVENT,
            ESP_EVENT_ANY_ID,
            &wifi_event_handler,
            NULL,
            NULL));

    ESP_ERROR_CHECK(
        esp_event_handler_instance_register(
            IP_EVENT,
            IP_EVENT_STA_GOT_IP,
            &wifi_event_handler,
            NULL,
            NULL));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };

    ESP_ERROR_CHECK(
        esp_wifi_set_mode(WIFI_MODE_STA));

    ESP_ERROR_CHECK(
        esp_wifi_set_config(
            WIFI_IF_STA,
            &wifi_config));

    ESP_ERROR_CHECK(
        esp_wifi_start());

    xEventGroupWaitBits(
        wifi_event_group,
        WIFI_CONNECTED_BIT,
        pdFALSE,
        pdFALSE,
        portMAX_DELAY);
}

/* ================= TIME SYNC ================= */

void obtain_time(void)
{
    esp_sntp_stop();

    esp_sntp_setoperatingmode(
        SNTP_OPMODE_POLL);

    esp_sntp_setservername(
        0,
        "time.google.com");

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

bool wifi_is_connected(void)
{
    EventBits_t bits =
        xEventGroupGetBits(
            wifi_event_group);

    return ((bits & WIFI_CONNECTED_BIT) != 0);
}