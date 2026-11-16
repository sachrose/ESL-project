#include "esl_button.h"
#include "app_timer.h"
#include "nrf_gpio.h"
#include "nrf_gpiote.h"
#include "nrfx_gpiote.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

bool esl_button_state_get(void);

void button_IRQHandler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action);

void debounce_IRQHandler(void * p_context);

void action_IRQHandler(void * p_context);

void esl_fsm_button(button_event_t event);

void state_sleep_handler(button_event_t event);

void state_1_click_handler(button_event_t event);

void state_hold_handler(button_event_t event);

void state_wait_click_handler(button_event_t event);

void state_2_click_handler(button_event_t event);

volatile static button_context_t button_context =
{
    .double_click_callback = NULL,
    .hold_start_callback = NULL,
    .hold_stop_callback = NULL,
    .state = BTN_STATE_SLEEP,
    .in_bouncing = false
};

static button_state_handler_t btn_state_handlers[BTN_STATE_COUNT] =
{
    state_sleep_handler,
    state_1_click_handler,
    state_hold_handler,
    state_wait_click_handler,
    state_2_click_handler
};

APP_TIMER_DEF(debounce);
APP_TIMER_DEF(action);

void apptimer2_init_for_button_handlers(void)
{
    app_timer_create(&debounce, APP_TIMER_MODE_SINGLE_SHOT, debounce_IRQHandler);   
    app_timer_create(&action, APP_TIMER_MODE_SINGLE_SHOT, action_IRQHandler);   
}

bool esl_button_state_get(void)
{
    bool pin_set = nrf_gpio_pin_read(BUTTON) ? true : false;
    return !pin_set;
}

void esl_button_gpiote_config(void)
{  
    nrfx_gpiote_in_config_t config = NRFX_GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);
    config.pull = BUTTON_PULL;

    nrfx_gpiote_in_init(BUTTON, &config, button_IRQHandler);
    nrfx_gpiote_in_event_enable(BUTTON, true);
}

void button_IRQHandler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    if (!button_context.in_bouncing)
    {
        app_timer_start(debounce, APP_TIMER_TICKS(DEBOUNCE_TIME_MS), NULL);
        button_context.in_bouncing = true;
    }
}

void debounce_IRQHandler(void * p_context)
{
    button_context.in_bouncing = false;

    if (esl_button_state_get())
    {
        esl_fsm_button(BTN_EVENT_PRESSED);
    }
    else
    {
        esl_fsm_button(BTN_EVENT_RELEASED);
    }
}

void action_IRQHandler(void * p_context)
{
    esl_fsm_button(BTN_EVENT_TIMER_TIMEOUT);
}

void esl_fsm_button(button_event_t event)
{
    btn_state_handlers[button_context.state](event);
}

void state_sleep_handler(button_event_t event)
{
    if (event == BTN_EVENT_PRESSED)
    {
        app_timer_start(action, APP_TIMER_TICKS(ACTION_TIME_MS), NULL);
        button_context.state = BTN_STATE_1_CLICK;
    }
}

void state_1_click_handler(button_event_t event)
{
    if (event == BTN_EVENT_RELEASED)
    {
        app_timer_stop(action);
        app_timer_start(action, APP_TIMER_TICKS(ACTION_TIME_MS), NULL);
        button_context.state = BTN_STATE_WAIT_CLICK;
    }

    if (event == BTN_EVENT_TIMER_TIMEOUT)
    {
        NRF_LOG_INFO("Hold: start!");
        button_context.state = BTN_STATE_HOLD;
        button_context.hold_start_callback();
    }
}

void state_hold_handler(button_event_t event)
{
    if (event == BTN_EVENT_RELEASED)
    {
        NRF_LOG_INFO("Hold: stop!");
        button_context.state = BTN_STATE_SLEEP;
        button_context.hold_stop_callback();
    }
}

void state_wait_click_handler(button_event_t event)
{
    if (event == BTN_EVENT_PRESSED)
    {
        app_timer_stop(action);
        button_context.state = BTN_STATE_2_CLICK;
    }
    else if (event == BTN_EVENT_TIMER_TIMEOUT)
    {
        NRF_LOG_INFO("Single click registered!");
        button_context.state = BTN_STATE_SLEEP;
    }
}

void state_2_click_handler(button_event_t event)
{
    if (event == BTN_EVENT_RELEASED)
    {
        NRF_LOG_INFO("Double click registered!");
        button_context.state = BTN_STATE_SLEEP;
        button_context.double_click_callback();
    }
}

void register_double_click_callback(button_callback_t double_click_callback)
{
    button_context.double_click_callback = double_click_callback;
}

void register_hold_start_callback(button_callback_t hold_start_callback)
{
    button_context.hold_start_callback = hold_start_callback;
}

void register_hold_stop_callback(button_callback_t hold_stop_callback)
{
    button_context.hold_stop_callback = hold_stop_callback;
}