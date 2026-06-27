#ifndef MYDATA_H
#define MYDATA_H

#define MQTT_URI  "mqtts://your-server.s1.eu.hivemq.cloud:8883"
#define MQTT_USER "YOUR_USERNAME"
#define MQTT_PASS "YOUR_PASSWORD"

#define MQTT_TOPIC_STATUS      "esp32/device1/status"
#define MQTT_TOPIC_CMD         "esp32/device1/led"
#define MQTT_TOPIC_OTA         "esp32/device1/ota"
#define MQTT_TOPIC_VERSION     "esp32/device1/version"
#define MQTT_TOPIC_TEMPERATURE "esp32/device1/temperature"

#define PWM_GPIO GPIO_NUM_2
#define OTA_LED_GPIO GPIO_NUM_17

//#define FW_VERSION "v1.0.1"
//#define FW_VERSION "v1.0.2"
//#define FW_VERSION "v1.0.3"
#define FW_VERSION "v1.0.4"


#endif
