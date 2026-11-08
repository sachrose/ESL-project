#ifndef LED_H
#define LED_H

#include <stdint.h>
#include "nrf_gpio.h"
#include "nrfx_glue.h"
#include "nrf_pwm.h"

#define LEDS_ACTIVE_STATE       0

#define ESL_LEDS_NUMBER         4

#define IDX_LED1_G              0
#define IDX_LED2_R              1
#define IDX_LED2_G              2
#define IDX_LED2_B              3

#define LED1_G         NRF_GPIO_PIN_MAP(0,6)
#define LED2_R         NRF_GPIO_PIN_MAP(0,8)
#define LED2_G         NRF_GPIO_PIN_MAP(1,9)
#define LED2_B         NRF_GPIO_PIN_MAP(0,12)

#define ESL_DEVICE_ID {1, 2, 1, 2}

#define ESL_LEDS_LIST { LED1_G, LED2_R, LED2_G, LED2_B }

#define NRFX_PWM_ESL_LED_CONFIG(led_idx)                                          \
{                                                                                 \
    .output_pins  = { led | NRFX_PWM_PIN_INVERTED,                                \
                      NRFX_PWM_PIN_NOT_USED,                                      \
                      NRFX_PWM_PIN_NOT_USED,                                      \
                      NRFX_PWM_PIN_NOT_USED },                                    \
    .irq_priority = NRFX_PWM_DEFAULT_CONFIG_IRQ_PRIORITY,                         \
    .base_clock   = (nrf_pwm_clk_t)NRFX_PWM_DEFAULT_CONFIG_BASE_CLOCK,            \
    .count_mode   = (nrf_pwm_mode_t)NRFX_PWM_DEFAULT_CONFIG_COUNT_MODE,           \
    .top_value    = NRFX_PWM_DEFAULT_CONFIG_TOP_VALUE,                            \
    .load_mode    = (nrf_pwm_dec_load_t)NRF_PWM_LOAD_INDIVIDUAL,                  \
    .step_mode    = (nrf_pwm_dec_step_t)NRFX_PWM_DEFAULT_CONFIG_STEP_MODE,        \
}

void esl_gpio_leds_init(void);

bool esl_gpio_led_state_get(uint32_t led_idx);

void esl_gpio_led_invert(uint32_t led_idx);

void esl_gpio_change_one_led_state(uint32_t led_idx, bool leds_active_state);

void esl_gpio_change_all_leds_state(bool leds_active_state);

void esl_gpio_led_blinking (uint32_t led_idx, uint32_t blinks_num);

void esl_gpio_led_blinking_mode_device_id (void);

__STATIC_INLINE void esl_delay_ms(uint32_t ms_time)
{
    if (ms_time == 0)
    {
        return;
    }

    do {
        NRFX_DELAY_US(1000);
    } while (--ms_time);
}

#endif