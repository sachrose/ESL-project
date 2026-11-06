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
    uint8_t led_idx;
    uint8_t count;
    bool button_pressed;
    bool in_bouncing;
    uint8_t clicks_num;
} blink_context_t;

void hardware_init();

void pwm_sequences_init(nrf_pwm_sequence_t *pwm_sequence_arr);

void debounce_IRQHandler(void * p_context);

void double_click_IRQHandler(void * p_context);

void button_IRQHandler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action);

void blink_iteration(void);

void led_blinking_mode_button(void);

#endif