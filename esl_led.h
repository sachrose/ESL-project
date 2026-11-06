#ifndef LED_H
#define LED_H

#include <stdint.h>
#include "nrf_gpio.h"
#include "nrfx_glue.h"

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

#define ESL_DEVICE_ID {6, 6, 1, 3}

#define ESL_LEDS_LIST { LED1_G, LED2_R, LED2_G, LED2_B }

#define ESL_LEDS_IDX_LIST { IDX_LED1_G, IDX_LED2_R, IDX_LED2_G, IDX_LED2_B }

#define esl_delay_us(us_time) NRFX_DELAY_US(us_time)

void esl_leds_init(void);

bool esl_led_state_get(uint32_t led_idx);

void esl_led_invert(uint32_t led_idx);

void esl_led_on(uint32_t led_idx);

void esl_led_off(uint32_t led_idx);

void esl_leds_on(void);

void esl_leds_off(void);

void led_blinking (uint32_t led_idx, uint32_t blinks_num);

void led_blinking_mode_device_id (void);

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