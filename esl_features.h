#ifndef ESL_FEATURES_H
#define ESL_FEATURES_H

#include <stdint.h>
#include "esl_led.h"

#define READY_TO_ON     1
#define READY_TO_OFF    0

typedef struct
{
    uint8_t how_much_is_the_fish[ESL_LEDS_NUMBER];  //рабочее название
    uint16_t delay_ms;
    uint8_t led_idx;
    uint8_t count;
    uint8_t state;
} blink_context_t;

void blink_iteration(void);

void led_blinking_mode_button(void);

#endif