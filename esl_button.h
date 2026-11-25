#ifndef ESL_BUTTON_H
#define ESL_BUTTON_H

#include <stdbool.h>

#define BUTTON_ACTIVE_STATE 0
#define BUTTON              NRF_GPIO_PIN_MAP(1,6)
#define BUTTON_PULL         NRF_GPIO_PIN_PULLUP

#define DEBOUNCE_TIME_MS    50
#define ACTION_TIME_MS      500

typedef enum
{
    BTN_STATE_SLEEP = 0,
    BTN_STATE_1_CLICK = 1,
    BTN_STATE_HOLD = 2,
    BTN_STATE_WAIT_CLICK = 3,
    BTN_STATE_2_CLICK = 4,
    BTN_STATE_COUNT = 5
} button_state_t;

typedef enum
{
    BTN_EVENT_PRESSED = 0,
    BTN_EVENT_RELEASED = 1,
    BTN_EVENT_TIMER_TIMEOUT = 2
} button_event_t;

typedef void (*button_callback_t)(void);

typedef struct
{
    button_callback_t double_click_callback;
    button_callback_t hold_start_callback;
    button_callback_t hold_stop_callback;
    button_state_t state;
    bool in_bouncing;
} button_context_t;

typedef void (*button_state_handler_t)(button_event_t event);

void esl_button_config(button_callback_t double_click_callback, button_callback_t hold_start_callback, button_callback_t hold_stop_callback);

#endif