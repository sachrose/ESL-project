#ifndef ESL_FEATURES_H
#define ESL_FEATURES_H

#include <stdint.h>
#include "esl_led.h"
//#include "nrf_gpiote.h"

void hardware_init(void);

void logs_init(void);

void led_mode_by_button(led_mode_t mode);

#endif