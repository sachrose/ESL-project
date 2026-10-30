#ifndef LED_H
#define LED_H

#include <stdint.h>
#include "nrf_gpio.h"
#include "nrfx_glue.h"

#define ESL_LEDS_NUMBER         4

#define LEDS_ACTIVE_STATE       0

#define LED_IDX_YELLOW_GREEN    0
#define LED_IDX_RED             1
#define LED_IDX_GREEN           2
#define LED_IDX_BLUE            3

#define LED1_G         NRF_GPIO_PIN_MAP(0,6)
#define LED2_R         NRF_GPIO_PIN_MAP(0,8)
#define LED2_G         NRF_GPIO_PIN_MAP(1,9)
#define LED2_B         NRF_GPIO_PIN_MAP(0,12)

#define ESL_LEDS_LIST { LED1_G, LED2_R, LED2_G, LED2_B }

#define ESL_LEDS_IDX_LIST { LED_IDX_YELLOW_GREEN, LED_IDX_RED, LED_IDX_GREEN, LED_IDX_BLUE }

#define esl_delay_us(us_time) NRFX_DELAY_US(us_time)

void esl_leds_init(void);

bool esl_led_state_get(uint32_t led_idx);

void esl_led_invert(uint32_t led_idx);

void esl_led_on(uint32_t led_idx);

void esl_led_off(uint32_t led_idx);

void esl_leds_on(void);

void esl_leds_off(void);

void led_blinking_mode_device_id (void);

void led_blinking (uint32_t led_idx, uint32_t blinks_num);

__STATIC_INLINE void esl_delay_ms(uint32_t ms_time)
{
    if (ms_time == 0)
    {
        return;
    }

    do {
        esl_delay_us(1000);
    } while (--ms_time);
}

#endif