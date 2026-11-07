#ifndef ESL_FEATURES_H
#define ESL_FEATURES_H

#include <stdint.h>
#include "esl_led.h"
#include "nrfx_gpiote.h"
#include "nrf_gpiote.h"
#include "nrf_pwm.h"

typedef struct
{
    uint8_t blinks[ESL_LEDS_NUMBER];
    uint16_t delay_ms;
    uint8_t led_idx;
    uint8_t count;
    bool ready_to_count;
} blink_context_t;

void hardware_init();

void esl_pwm_init(uint8_t led);

void sequence_init(void);

void esl_pwm_led_smooth_blinking(uint8_t led_idx, uint16_t blinks_num);

void esl_pwm_led_smooth_blinking_mode_device_id(void);

void debounce_IRQHandler(void * p_context);

void double_click_IRQHandler(void * p_context);

void button_IRQHandler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action);

void blink_iteration(void);

void led_blinking_mode_button(void);

#endif