#ifndef ESL_FEATURES_H
#define ESL_FEATURES_H

#include <stdint.h>
#include "esl_led.h"

typedef struct
{
    uint8_t blinks[ESL_LEDS_NUMBER];
    uint16_t delay_ms;
    uint8_t led_idx;
    uint8_t count;
    bool ready_to_count;
} blink_context_t;

void hardware_init();

void blink_iteration(void);

void led_blinking_mode_button(void);

#endif