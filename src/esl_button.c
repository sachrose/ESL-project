#include "esl_button.h"
#include "app_timer.h"
#include "nrf_gpio.h"
#include "nrf_gpiote.h"
#include "nrfx_gpiote.h"
#include "esl_log.h"

static button_context_t button_context =
{
    .single_click_callback = NULL,
    .double_click_callback = NULL,
    .hold_start_callback   = NULL,
    .hold_stop_callback    = NULL,
    .state                 = BTN_STATE_SLEEP,
};

APP_TIMER_DEF(debounce);
APP_TIMER_DEF(action);

static void state_sleep_handler(button_event_t event)
{
    if (event == BTN_EVENT_PRESSED)
    {
        app_timer_start(action, APP_TIMER_TICKS(ACTION_TIME_MS), NULL);
        button_context.state = BTN_STATE_1_CLICK;
    }
}

static void state_1_click_handler(button_event_t event)
{
    if (event == BTN_EVENT_RELEASED)
    {
        app_timer_stop(action);
        app_timer_start(action, APP_TIMER_TICKS(ACTION_TIME_MS), NULL);
        button_context.state = BTN_STATE_WAIT_CLICK;
    }

    if (event == BTN_EVENT_TIMER_TIMEOUT)
    {
        esl_log_info("Hold: start!");
        button_context.state = BTN_STATE_HOLD;

        if (button_context.hold_start_callback != NULL)
        {
            button_context.hold_start_callback();
        }        
    }
}

static void state_hold_handler(button_event_t event)
{
    if (event == BTN_EVENT_RELEASED)
    {
        esl_log_info("Hold: stop!");
        button_context.state = BTN_STATE_SLEEP;

        if (button_context.hold_stop_callback != NULL)
        {
            button_context.hold_stop_callback();
        }        
    }
}

static void state_wait_click_handler(button_event_t event)
{
    if (event == BTN_EVENT_PRESSED)
    {
        app_timer_stop(action);
        button_context.state = BTN_STATE_2_CLICK;
    }
    else if (event == BTN_EVENT_TIMER_TIMEOUT)
    {
        esl_log_info("Single click registered!");
        button_context.state = BTN_STATE_SLEEP;

        if (button_context.single_click_callback != NULL)
        {
            button_context.single_click_callback();
        }    
    }
}

static void state_2_click_handler(button_event_t event)
{
    if (event == BTN_EVENT_RELEASED)
    {
        esl_log_info("Double click registered!");
        button_context.state = BTN_STATE_SLEEP;

        if (button_context.double_click_callback != NULL)
        {
            button_context.double_click_callback();
        }        
    }
}

static button_state_handler_t btn_state_handlers[BTN_STATE_COUNT] =
{
    state_sleep_handler,
    state_1_click_handler,
    state_hold_handler,
    state_wait_click_handler,
    state_2_click_handler
};

static void esl_fsm_button(button_event_t event)
{
    btn_state_handlers[button_context.state](event);
}

static bool esl_button_state_get(void)
{
    bool pin_set = nrf_gpio_pin_read(BUTTON) ? true : false;
    return !pin_set;
}

static void button_IRQHandler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    nrfx_gpiote_in_event_disable(BUTTON);

    app_timer_start(debounce, APP_TIMER_TICKS(DEBOUNCE_TIME_MS), NULL);
}

static void debounce_IRQHandler(void * p_context)
{
    nrfx_gpiote_in_event_enable(BUTTON, true);

    if (esl_button_state_get())
    {
        esl_fsm_button(BTN_EVENT_PRESSED);
    }
    else
    {
        esl_fsm_button(BTN_EVENT_RELEASED);
    }
}

static void action_IRQHandler(void * p_context)
{
    esl_fsm_button(BTN_EVENT_TIMER_TIMEOUT);
}

void esl_button_config(button_callback_t single_click_callback,
                       button_callback_t double_click_callback,
                       button_callback_t hold_start_callback,
                       button_callback_t hold_stop_callback)
{  
    button_context.single_click_callback = single_click_callback;
    button_context.double_click_callback = double_click_callback;
    button_context.hold_start_callback   = hold_start_callback;
    button_context.hold_stop_callback    = hold_stop_callback;

    app_timer_create(&debounce, APP_TIMER_MODE_SINGLE_SHOT, debounce_IRQHandler);   
    app_timer_create(&action, APP_TIMER_MODE_SINGLE_SHOT, action_IRQHandler);  

    nrfx_gpiote_init();
    
    nrfx_gpiote_in_config_t config = NRFX_GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);
    config.pull = BUTTON_PULL;

    nrfx_gpiote_in_init(BUTTON, &config, button_IRQHandler);
    nrfx_gpiote_in_event_enable(BUTTON, true);
}