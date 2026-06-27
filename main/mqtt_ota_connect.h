#ifndef MQTT_OTA_CONNECT_H_
#define MQTT_OTA_CONNECT_H_

#include <stdbool.h>
#include "mqtt_client.h"

extern bool mqtt_connected;
extern bool ota_running;


void mqtt_app_start(void);
void start_ota_update();
void validate_new_firmware(void);

esp_mqtt_client_handle_t mqtt_get_client(void);

#endif