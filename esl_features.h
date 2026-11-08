#ifndef ESL_FEATURES_H
#define ESL_FEATURES_H

#include <stdint.h>
#include "esl_led.h"
#include "nrfx_gpiote.h"
#include "nrf_gpiote.h"
#include "nrf_pwm.h"
#include "nrfx_systick.h"

typedef struct
{
    uint8_t clicks_count;
    bool in_bouncing;
    bool button_button;
} button_context_t;

typedef struct
{
    uint8_t blinks[ESL_LEDS_NUMBER];
    uint16_t delay_ms;
    uint8_t led_idx;
    uint8_t count;
    bool ready_to_count;    
} blink_context_t;

typedef struct
{
    uint8_t blinks[ESL_LEDS_NUMBER];
    uint8_t led_pins[ESL_LEDS_NUMBER];
    nrfx_systick_state_t systick_state;
    uint8_t led_idx;
    uint8_t count;
    uint8_t current_step;
} smooth_blink_context_t;

void hardware_init(void);

void logs_init(void);

void esl_button_gpiote_config(void);

void button_IRQHandler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action);

void debounce_IRQHandler(void * p_context);

void double_click_IRQHandler(void * p_context);

void blink_iteration(void);

void led_blinking_mode_button(void);

void esl_pwm_init(uint8_t led_pin);

void esl_pwm_seq_values_init(void);

void led_smooth_blink_iteration(void);

void led_smooth_blinking_mode_button(void);

#endif