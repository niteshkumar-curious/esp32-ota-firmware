#ifndef COMMON_HEADER_H_
#define COMMON_HEADER_H_


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

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
#include "driver/ledc.h"


#include "mqtt_client.h"
#include "esp_crt_bundle.h"

#include "esp_https_ota.h"
#include "esp_ota_ops.h"

#include "cJSON.h"



#endif