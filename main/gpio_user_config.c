#include "gpio_user_config.h"
#include "common_header.h"
#include "mydata.h"



/*                    gpio init                   */

void gpio_init(void)
{
    gpio_config_t led_conf = {
        .pin_bit_mask =
            (1ULL << OTA_LED_GPIO),
        .mode = GPIO_MODE_OUTPUT,
    };

    gpio_config(&led_conf);

    
}

void pwm_init(void)
{
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution= LEDC_TIMER_13_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = 5000,
        .clk_cfg = LEDC_AUTO_CLK
    }; 
    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel = {
        .gpio_num = PWM_GPIO,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .intr_type  = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint =0
    };

    ledc_channel_config(&ledc_channel);

}

void pwm_set_duty(uint32_t duty_cycle)
{
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty_cycle);
        ledc_update_duty(
            LEDC_LOW_SPEED_MODE,
            LEDC_CHANNEL_0);
}
