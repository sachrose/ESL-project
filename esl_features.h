#ifndef ESL_FEATURES_H
#define ESL_FEATURES_H

#include <stdint.h>
#include "esl_led.h"

#if defined(BLINK_MODE_SIMPLE_ENABLED) && defined(BLINK_MODE_SMOOTH_ENABLED)
#error Choose only one mode in Makefile!
#endif

#ifdef BLINK_MODE_SIMPLE_ENABLED
void hardware_simple_init(void);
#endif

#ifdef BLINK_MODE_SMOOTH_ENABLED
void hardware_smooth_init(void);
#endif

void logs_init(void);

void led_mode_by_button(void);

#endif