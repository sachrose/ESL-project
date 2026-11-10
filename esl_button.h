#ifndef ESL_BUTTON_H
#define ESL_BUTTON_H

#include <stdint.h>
#include "nrf_gpio.h"
#include "nrf_gpiote.h"
#include "nrfx_gpiote.h"

#define BUTTON          NRF_GPIO_PIN_MAP(1,6)
#define BUTTON_PULL     NRF_GPIO_PIN_PULLUP

#define BUTTON_ACTIVE_STATE 0

typedef struct
{
    uint8_t clicks_count;
    bool in_bouncing;
    volatile bool button_button;
} button_context_t;

void esl_button_init(void);

bool esl_button_state_get(void);

bool esl_button_button_get(void);

void esl_button_gpiote_config(void);

void apptimer2_init_for_button_handlers(void);

void button_IRQHandler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action);

void debounce_IRQHandler(void * p_context);

void double_click_IRQHandler(void * p_context);

#endif