#ifndef ESL_FEATURES_H
#define ESL_FEATURES_H

#include <stdint.h>
#include "esl_led.h"

typedef struct
{
    uint8_t how_much_is_the_fish[ESL_LEDS_NUMBER];  //рабочее название
    uint8_t led_idx;
    uint8_t count;
} blink_context_t;

void blink_iteration(void);

void led_blinking_mode_button(void);

#endif