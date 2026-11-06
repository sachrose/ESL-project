#include "esl_features.h"
#include "esl_button.h"
#include "esl_led.h"
#include "nrfx_gpiote.h"
#include "nrf_gpiote.h"
#include "nrfx_pwm.h"
#include "nrf_drv_clock.h"
#include "nrf_log.h"
#include "app_timer.h"

APP_TIMER_DEF(debounce);
APP_TIMER_DEF(double_click);

static nrf_pwm_sequence_t pwm_sequence_arr;

static uint16_t values[] = {0, 200, 400, 600, 800, 1000, 800, 600, 400, 200, 0};

static const nrfx_pwm_t pwm = NRFX_PWM_INSTANCE(0);

static nrfx_pwm_config_t pwm_cfg = NRFX_PWM_DEFAULT_CONFIG;

volatile blink_context_t blink_context = 
{
    .blinks = {6, 6, 1, 3},
    .led_idx = IDX_LED1_G,
    .count = 6,
    .button_pressed = false,
    .in_bouncing = false,
    .clicks_num = 0
};

void hardware_init()
{
    nrf_drv_clock_init();

    nrf_drv_clock_lfclk_request(NULL);
    while (!nrf_drv_clock_lfclk_is_running())
    {
        /* Just waiting until the lf clock starts up */
    }
    NRF_LOG_DEBUG("Init the LF clock for USB - End");
       
    app_timer_init();   
    app_timer_create(&debounce, APP_TIMER_MODE_SINGLE_SHOT, debounce_IRQHandler);
    app_timer_create(&double_click, APP_TIMER_MODE_SINGLE_SHOT, double_click_IRQHandler);
    NRF_LOG_DEBUG("Init the APP_TIMER2 - End");

    nrfx_gpiote_init();
    esl_button_gpiote_config();

    nrfx_gpiote_in_event_enable(BUTTON, true);
    NRF_LOG_DEBUG("Init GPIOTE - End"); 

    pwm_cfg.output_pins[0] = LED1_G | NRFX_PWM_PIN_INVERTED;
    pwm_cfg.output_pins[1] = LED2_R | NRFX_PWM_PIN_INVERTED;
    pwm_cfg.output_pins[2] = LED2_G | NRFX_PWM_PIN_INVERTED;
    pwm_cfg.output_pins[3] = LED2_B | NRFX_PWM_PIN_INVERTED;
    nrfx_pwm_init(&pwm, &pwm_cfg, NULL);    
    pwm_sequences_init(&pwm_sequence_arr);
    NRF_LOG_DEBUG("Init PWM - End");
}

void esl_button_gpiote_config(void)
{  
    nrfx_gpiote_in_config_t context = NRFX_GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);
    context.pull = BUTTON_PULL;

    nrfx_gpiote_in_init(BUTTON, &context, button_IRQHandler);
}

void debounce_IRQHandler(void * p_context)
{
    if (blink_context.clicks_num == 0)
    {
        app_timer_start(double_click, APP_TIMER_TICKS(300), NULL);
        blink_context.in_bouncing = false;
        blink_context.clicks_num = 1;
    }
    else
    {
        app_timer_stop(double_click);
        blink_context.clicks_num = 0;
        blink_context.button_pressed = !blink_context.button_pressed;
    }
}

void double_click_IRQHandler(void * p_context)
{
    blink_context.button_pressed = false;
    blink_context.clicks_num = 0;
}

void button_IRQHandler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    if (!blink_context.in_bouncing)
    {
        app_timer_start(debounce, APP_TIMER_TICKS(50), NULL);
        blink_context.in_bouncing = true;
    }
}

void pwm_sequences_init(nrf_pwm_sequence_t *pwm_sequence_arr)
{
    pwm_sequence_arr->values.p_common = values;
    pwm_sequence_arr->length = NRF_PWM_VALUES_LENGTH(values);
    pwm_sequence_arr->repeats = blink_context.count;
    pwm_sequence_arr->end_delay = 0;
}

void blink_iteration(void)
{
    nrfx_pwm_simple_playback(&pwm, &pwm_sequence_arr, 1, 0);
}

void led_blinking_mode_button(void)
{
    if (blink_context.button_pressed)
    {
        blink_iteration();
    }  
} 
