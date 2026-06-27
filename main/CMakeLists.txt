idf_component_register(
    SRCS "adc_cal_us.c" "task_handling.c" "gpio_user_config.c" "mqtt_ota_connect.c" "wifi.c" "main.c"
    INCLUDE_DIRS "."
    REQUIRES
        esp_wifi
        esp_event
        esp_netif
        nvs_flash
        mbedtls
        mqtt
        esp_https_ota
        app_update
        driver
        json
        esp_adc
)