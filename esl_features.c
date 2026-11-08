#include "esl_features.h"
#include "esl_button.h"
#include "esl_led.h"
#include "nrfx_gpiote.h"
#include "nrf_gpiote.h"
#include "nrfx_pwm.h"
#include "nrf_drv_pwm.h"
#include "nrf_drv_clock.h"
#include "nrf_log.h"
#include "app_timer.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrfx_systick.h"

#define PWM_SEQ_STEPS       50
#define PWM_STEPS_ARR_SIZE  (PWM_SEQ_STEPS & 1 ? PWM_SEQ_STEPS : (PWM_SEQ_STEPS + 1))
#define PWM_ONE_STEP_DUTY   (NRFX_PWM_DEFAULT_CONFIG_TOP_VALUE / PWM_SEQ_STEPS)
#define DUTY_DURATION_US       20000

uint32_t esl_leds_list[ESL_LEDS_NUMBER] = ESL_LEDS_LIST;
uint8_t blinks_num[ESL_LEDS_NUMBER] = ESL_DEVICE_ID;

APP_TIMER_DEF(debounce);
APP_TIMER_DEF(double_click);

static nrfx_pwm_t pwm_0 = NRFX_PWM_INSTANCE(0);

static nrf_pwm_values_common_t pwm_seq_0_values[PWM_STEPS_ARR_SIZE ];
static nrf_pwm_sequence_t pwm_seq_0 = 
{
    .values.p_common = pwm_seq_0_values,
    .length = 1,
    .repeats = 0,
    .end_delay = 0
};

static button_context_t button_context =
{
    .clicks_count = 0,
    .in_bouncing = false,
    .button_button = false
};

static blink_context_t blink_context = 
{
    .blinks = ESL_DEVICE_ID,
    .delay_ms = 0,
    .led_idx = 0,
    .count = 6,
    .ready_to_count = true
};

static smooth_blink_context_t smooth_blink_context = 
{
    .blinks = ESL_DEVICE_ID,
    .led_pins = ESL_LEDS_LIST,
    .systick_state.time = 0,
    .led_idx = 3,
    .count = 0,
    .current_step = 0
};

void hardware_init()
{
    logs_init();
    nrfx_systick_init();
    nrfx_systick_get(&smooth_blink_context.systick_state);
    NRF_LOG_INFO(">hardware_init()");       
    NRF_LOG_DEBUG("Init the LF clock for USB - End");
   
    app_timer_init();   
    app_timer_create(&debounce, APP_TIMER_MODE_SINGLE_SHOT, debounce_IRQHandler);
    app_timer_create(&double_click, APP_TIMER_MODE_SINGLE_SHOT, double_click_IRQHandler);
    NRF_LOG_DEBUG("Init the APP_TIMER2 - End");

    nrfx_gpiote_init();
    esl_button_gpiote_config();
    nrfx_gpiote_in_event_enable(BUTTON, true);
    NRF_LOG_DEBUG("Init GPIOTE - End"); 

    esl_pwm_seq_values_init();
    esl_pwm_init(LED2_B);
    NRF_LOG_DEBUG("Init PWM - End");
    NRF_LOG_INFO("<hardware_init()");
}

void logs_init(void)
{
    ret_code_t ret = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(ret);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}

void esl_button_gpiote_config(void)
{  
    nrfx_gpiote_in_config_t config = NRFX_GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);
    config.pull = BUTTON_PULL;

    nrfx_gpiote_in_init(BUTTON, &config, button_IRQHandler);
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

void blink_iteration(void)
{
    if (blink_context.delay_ms == 0)
    {
        esl_gpio_led_invert(blink_context.led_idx);
        blink_context.ready_to_count = !blink_context.ready_to_count;
        blink_context.delay_ms = 500;
    }
    
    while (blink_context.delay_ms > 0 && button_context.button_button)    
    {
        esl_delay_ms(1);
        blink_context.delay_ms--;
    }

    if (blink_context.delay_ms > 0)
    {
        return;
    }  

    if (blink_context.count > 0 && blink_context.ready_to_count)
    {
        blink_context.count--;
    }
    
    if (blink_context.count == 0)
    {
        blink_context.led_idx = (blink_context.led_idx + 1) % ESL_LEDS_NUMBER;
        blink_context.count = blink_context.blinks[blink_context.led_idx];
    }
}

void led_blinking_mode_button(void)
{
    if (button_context.button_button)
    {
        blink_iteration();
    }  
} 

void esl_pwm_init(uint8_t led_pin)
{   
    nrfx_pwm_config_t pwm_0_cfg = 
    {
        .output_pins =
        {
            led_pin | NRFX_PWM_PIN_INVERTED,
            NRF_DRV_PWM_PIN_NOT_USED,
            NRF_DRV_PWM_PIN_NOT_USED,
            NRF_DRV_PWM_PIN_NOT_USED
        },
        .irq_priority = NRFX_PWM_DEFAULT_CONFIG_IRQ_PRIORITY,
        .base_clock   = (nrf_pwm_clk_t)NRFX_PWM_DEFAULT_CONFIG_BASE_CLOCK,
        .count_mode   = NRFX_PWM_DEFAULT_CONFIG_COUNT_MODE,
        .top_value    = NRFX_PWM_DEFAULT_CONFIG_TOP_VALUE,
        .load_mode    = NRFX_PWM_DEFAULT_CONFIG_LOAD_MODE,
        .step_mode    = (nrf_pwm_dec_step_t)NRFX_PWM_DEFAULT_CONFIG_STEP_MODE,
    };
    
    
    /*NRFX_PWM_DEFAULT_CONFIG;
    pwm_0_cfg.output_pins[0] = (esl_leds_list[0] | NRFX_PWM_PIN_INVERTED);
    pwm_0_cfg.output_pins[1] = (esl_leds_list[1] | NRFX_PWM_PIN_INVERTED);
    pwm_0_cfg.output_pins[2] = (esl_leds_list[2] | NRFX_PWM_PIN_INVERTED);
    pwm_0_cfg.output_pins[3] = (esl_leds_list[3] | NRFX_PWM_PIN_INVERTED);
    pwm_0_cfg.load_mode = NRF_PWM_LOAD_INDIVIDUAL;
    */

    nrfx_pwm_init(&pwm_0, &pwm_0_cfg, NULL); 
}

void esl_pwm_seq_values_init(void)
{
    for (uint16_t i = 0; i < PWM_STEPS_ARR_SIZE; i++)
    {
        uint16_t value = (i <= PWM_STEPS_ARR_SIZE / 2) ? i : (PWM_STEPS_ARR_SIZE - 1 - i);
        pwm_seq_0_values[i] = value * PWM_ONE_STEP_DUTY;
    }
}

void led_smooth_blink_iteration(void)
{   
    if (smooth_blink_context.count == 0)
    {
        smooth_blink_context.led_idx = (smooth_blink_context.led_idx + 1) % ESL_LEDS_NUMBER;
        smooth_blink_context.count = smooth_blink_context.blinks[smooth_blink_context.led_idx];

        nrfx_pwm_uninit(&pwm_0);
        esl_pwm_init(smooth_blink_context.led_pins[smooth_blink_context.led_idx]);
        nrfx_systick_get(&smooth_blink_context.systick_state);
        NRF_LOG_DEBUG("start blinking by LED_%d, count %d", smooth_blink_context.led_idx, smooth_blink_context.count);
    }

    if (smooth_blink_context.count > 0)
    {
        if (nrfx_systick_test(&smooth_blink_context.systick_state, DUTY_DURATION_US))
        {
            smooth_blink_context.current_step = (smooth_blink_context.current_step + 1) % PWM_STEPS_ARR_SIZE;
            pwm_seq_0.values.p_common = &pwm_seq_0_values[smooth_blink_context.current_step];
            
            nrfx_systick_get(&smooth_blink_context.systick_state);
            nrfx_pwm_simple_playback(&pwm_0, &pwm_seq_0, 1, NRFX_PWM_FLAG_LOOP);

            if (smooth_blink_context.current_step == (PWM_STEPS_ARR_SIZE - 1))
            {
                smooth_blink_context.count--;
            }
        }        
    }
}

void led_smooth_blinking_mode_button(void)
{
    if (button_context.button_button)
    {
        led_smooth_blink_iteration();
    }
}