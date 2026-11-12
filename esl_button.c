#include "esl_button.h"
#include "app_timer.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

APP_TIMER_DEF(debounce);
APP_TIMER_DEF(action);

static button_context_t button_context =
{
    .state = BTN_STATE_SLEEP,
    .in_bouncing = false,
    .button_button = false
};

void esl_button_init(void)
{
    nrf_gpio_cfg_input(BUTTON, BUTTON_PULL);
}

bool esl_button_state_get(void)
{
    bool pin_set = nrf_gpio_pin_read(BUTTON) ? true : false;
    return !pin_set;
}

bool esl_button_button_get(void)
{
    return button_context.button_button;
}

void esl_button_gpiote_config(void)
{  
    nrfx_gpiote_in_config_t config = NRFX_GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);
    config.pull = BUTTON_PULL;

    nrfx_gpiote_in_init(BUTTON, &config, button_IRQHandler);
    nrfx_gpiote_in_event_enable(BUTTON, true);
}

void apptimer2_init_for_button_handlers(void)
{
    app_timer_create(&debounce, APP_TIMER_MODE_SINGLE_SHOT, debounce_IRQHandler);   
    app_timer_create(&action, APP_TIMER_MODE_SINGLE_SHOT, action_IRQHandler);   
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
    switch (button_context.state)
    {
        case BTN_STATE_SLEEP:
            if (event == BTN_EVENT_PRESSED)
            {
                button_context.state = BTN_STATE_1_CLICK;
            }
            break;

        case BTN_STATE_1_CLICK:
            if (event == BTN_EVENT_RELEASED)
            {
                app_timer_start(action, APP_TIMER_TICKS(DOUBLE_CLICK_TIME_MS), NULL);
                button_context.state = BTN_STATE_WAIT_CLICK;
            }
            break;

        case BTN_STATE_WAIT_CLICK:
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
            break;

        case BTN_STATE_2_CLICK:
            if (event == BTN_EVENT_RELEASED)
            {
                button_context.button_button = !button_context.button_button;
                NRF_LOG_INFO("Double click registered!");
                //esl_fsm_led();
                button_context.state = BTN_STATE_SLEEP;
            }
            break;

        default:
            button_context.state = BTN_STATE_SLEEP;
            break;
    }
}