#include "esl_button.h"
#include "app_timer.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

APP_TIMER_DEF(debounce);
APP_TIMER_DEF(double_click);

button_context_t button_context =
{
    .clicks_count = 0,
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
}

void apptimer2_init_for_button_handlers(void)
{
    app_timer_create(&debounce, APP_TIMER_MODE_SINGLE_SHOT, debounce_IRQHandler);   
    app_timer_create(&double_click, APP_TIMER_MODE_SINGLE_SHOT, double_click_IRQHandler);   
}

void button_IRQHandler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    if (!button_context.in_bouncing)
    {
        app_timer_start(debounce, APP_TIMER_TICKS(200), NULL);
        button_context.in_bouncing = true;
    }
}

void debounce_IRQHandler(void * p_context)
{
    if (button_context.clicks_count == 0)
    {
        app_timer_start(double_click, APP_TIMER_TICKS(500), NULL);
        button_context.clicks_count = 1;
    }
    else
    {
        app_timer_stop(double_click);
        button_context.clicks_count = 0;
        button_context.button_button = !button_context.button_button;
        NRF_LOG_INFO("Double click registered!");
    }

    button_context.in_bouncing = false;
}

void double_click_IRQHandler(void * p_context)
{
    button_context.clicks_count = 0;
}