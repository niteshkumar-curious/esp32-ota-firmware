#ifndef GPIO_USER_CONFIG_H_
#define GPIO_USER_CONFIG_H_
#include <stdint.h>

void gpio_init(void);
void pwm_init(void);
void pwm_set_duty(uint32_t duty_cycle);

#endif