#ifndef ESL_FEATURES_H
#define ESL_FEATURES_H

#include <stdint.h>
#include "esl_led.h"
#include "nrfx_gpiote.h"
#include "nrf_gpiote.h"
#include "nrf_pwm.h"

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
    uint8_t led_idx;
    uint8_t count;
} smooth_blink_context_t;

void hardware_init(void);

void logs_init(void);

void esl_button_gpiote_config(void);

void button_IRQHandler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action);

void debounce_IRQHandler(void * p_context);

void double_click_IRQHandler(void * p_context);

void blink_iteration(void);

void led_blinking_mode_button(void);

void esl_pwm_init(void);

void sequence_init(uint16_t channel, uint16_t duty);

void up_and_dowm();

void esl_pwm_led_smooth_blinking();




#endif