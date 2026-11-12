#ifndef ESL_FEATURES_H
#define ESL_FEATURES_H

#include <stdint.h>
#include "esl_led.h"

typedef enum
{
    VIEWING = 0,
    EDITING_H = 1,
    EDITING_S = 2,
    EDITING_V = 3,
} led_state_t;

void hardware_smooth_init(void);

void logs_init(void);

void led_mode_by_button(void);

#endif