#ifndef ESL_BUTTON_H
#define ESL_BUTTON_H

#include <stdint.h>
#include "nrf_gpio.h"

#define BUTTON          NRF_GPIO_PIN_MAP(1,6)
#define BUTTON_PULL     NRF_GPIO_PIN_PULLUP

#define BUTTON_ACTIVE_STATE 0

void esl_button_init(void);

bool esl_button_state_get(void);

#endif