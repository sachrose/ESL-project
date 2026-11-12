#ifndef ESL_BUTTON_H
#define ESL_BUTTON_H

#include <stdint.h>
#include "nrf_gpio.h"
#include "nrf_gpiote.h"
#include "nrfx_gpiote.h"

#define BUTTON          NRF_GPIO_PIN_MAP(1,6)
#define BUTTON_PULL     NRF_GPIO_PIN_PULLUP

#define BUTTON_ACTIVE_STATE   0


#define DEBOUNCE_TIME_MS      50
#define DOUBLE_CLICK_TIME_MS  500

typedef enum
{
    BTN_STATE_SLEEP = 0,
    BTN_STATE_1_CLICK = 1,
    BTN_STATE_2_CLICK = 2,
    BTN_STATE_WAIT_CLICK = 3,
} button_state_t;

typedef enum
{
    BTN_EVENT_PRESSED = 0,
    BTN_EVENT_RELEASED = 1,
    BTN_EVENT_TIMER_TIMEOUT = 2
} button_event_t;

typedef struct
{
    button_state_t state;
    bool in_bouncing;
    bool button_button;
} button_context_t;

void esl_button_init(void);

bool esl_button_state_get(void);

bool esl_button_button_get(void);

void esl_button_gpiote_config(void);

void apptimer2_init_for_button_handlers(void);

void button_IRQHandler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action);

void debounce_IRQHandler(void * p_context);

void action_IRQHandler(void * p_context);

void esl_fsm_button(button_event_t event);

#endif