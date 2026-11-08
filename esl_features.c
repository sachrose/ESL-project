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

#define PWM_SEQ_STEPS       100
#define PWM_ONE_STEP_DUTY   (NRFX_PWM_DEFAULT_CONFIG_TOP_VALUE / PWM_SEQ_STEPS)

//uint32_t esl_leds_list[ESL_LEDS_NUMBER] = ESL_LEDS_LIST;

APP_TIMER_DEF(debounce);
APP_TIMER_DEF(double_click);

//static nrfx_pwm_t pwm_0 = NRFX_PWM_INSTANCE(0);
//static nrf_pwm_values_individual_t pwm_seq_0_values[PWM_SEQ_STEPS * 2];

//uint8_t blinks_num[ESL_LEDS_NUMBER] = ESL_DEVICE_ID;
//static nrf_pwm_values_individual_t pwm_seq_1_values[PWM_SEQ_STEPS];

static button_context_t button_context =
{
    .clicks_count = 0,
    .in_bouncing = false,
    .button_button = false
};

static blink_context_t blink_context = 
{
    .blinks = {6, 6, 1, 3},
    .delay_ms = 0,
    .led_idx = IDX_LED1_G,
    .count = 6,
    .ready_to_count = true
};
/*
static smooth_blink_context_t smooth_blink_context = 
{
    .blinks = {6, 6, 1, 3},
    .led_idx = IDX_LED1_G,
    .count = 6
};
*/

void hardware_init()
{    
    
    logs_init();
    
    NRF_LOG_INFO(">hardware_init()");    
    esl_gpio_leds_init();    
    NRF_LOG_DEBUG("Init the LF clock for USB - End\nInit leds - End");
   
    app_timer_init();   
    app_timer_create(&debounce, APP_TIMER_MODE_SINGLE_SHOT, debounce_IRQHandler);
    app_timer_create(&double_click, APP_TIMER_MODE_SINGLE_SHOT, double_click_IRQHandler);
    NRF_LOG_DEBUG("Init the APP_TIMER2 - End");

    nrfx_gpiote_init();
    esl_button_gpiote_config();
    nrfx_gpiote_in_event_enable(BUTTON, true);
    NRF_LOG_DEBUG("Init GPIOTE - End"); 
/*
    esl_pwm_init();
    NRF_LOG_DEBUG("Init PWM - End");
    NRF_LOG_INFO("<hardware_init()");

    */
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
/*
void esl_pwm_init(void)
{
    nrfx_pwm_config_t pwm_0_cfg = NRFX_PWM_DEFAULT_CONFIG;
    pwm_0_cfg.output_pins[0] = (esl_leds_list[0] | NRFX_PWM_PIN_INVERTED);
    pwm_0_cfg.output_pins[1] = (esl_leds_list[1] | NRFX_PWM_PIN_INVERTED);
    pwm_0_cfg.output_pins[2] = (esl_leds_list[2] | NRFX_PWM_PIN_INVERTED);
    pwm_0_cfg.output_pins[3] = (esl_leds_list[3] | NRFX_PWM_PIN_INVERTED);
    pwm_0_cfg.load_mode = NRF_PWM_LOAD_INDIVIDUAL;

    nrfx_pwm_init(&pwm_0, &pwm_0_cfg, NULL); 
}

void sequence_init(uint8_t channel, uint16_t duty)
{
    switch (channel)
    {
    case 0:
        pwm_seq_0_values->channel_0 = duty;
        break;
    case 1:
        pwm_seq_0_values->channel_1 = duty;
        break;
    case 2:
        pwm_seq_0_values->channel_2 = duty;
        break;
    case 3:
        pwm_seq_0_values->channel_3 = duty;
        break;
    
    default:
        NRF_LOG_DEBUG("Sequence_init: Something went wrong");   
    }

    nrf_pwm_sequence_t pwm_0_seq_0 =
    {
        .values.p_individual = pwm_seq_0_values,
        .length = NRF_PWM_VALUES_LENGTH(pwm_seq_0_values),
        .repeats = 0,
        .end_delay = 0
    };

    //static nrf_pwm_sequence_t pwm_0_seq_1;  
    nrfx_pwm_simple_playback(&pwm_0, &pwm_0_seq_0, 1, 0);
}

void up_and_dowm(uint8_t led_idx)
{
    NRF_LOG_INFO("Light on");
    for (uint32_t i = 0; i < NRFX_PWM_DEFAULT_CONFIG_TOP_VALUE; i += PWM_ONE_STEP_DUTY)
    {
        sequence_init(led_idx, i);
    }
    
    NRF_LOG_INFO("Light off");
    for (uint32_t i = NRFX_PWM_DEFAULT_CONFIG_TOP_VALUE; i >= 0; i -= PWM_ONE_STEP_DUTY)
    {
        sequence_init(led_idx, i);
    }
    sequence_init(led_idx, 0);


}

void esl_pwm_led_smooth_blinking(uint8_t led_idx)
{
    for (uint16_t i = 0; i < blinks_num[i]; i++)
    {
        up_and_dowm(led_idx);
    }
}
*/
    
   





 /*
    
    for (uint16_t i = 0; i < PWM_SEQ_STEPS; i++)
    {
        uint16_t light_power = ((i * NRFX_PWM_DEFAULT_CONFIG_TOP_VALUE) / PWM_SEQ_STEPS);
        pwm_seq_0_values[i].channel_0 = light_power;
        pwm_seq_1_values[PWM_SEQ_STEPS -1 - i].channel_0 = light_power;
    }
       
    pwm_sequence_0.values.p_individual = pwm_seq_0_values;
    pwm_sequence_1.values.p_individual = pwm_seq_1_values;
   
    pwm_sequence_0.length = pwm_sequence_1.length = PWM_SEQ_STEPS;
    pwm_sequence_0.repeats = pwm_sequence_1.repeats = 2;
    pwm_sequence_0.end_delay = pwm_sequence_1.end_delay = 0;

    */





















/*

void esl_pwm_init(uint8_t led)
{
    nrfx_pwm_config_t pwm_0_cfg = NRFX_PWM_ESL_LED_CONFIG(led_idx);
    nrfx_pwm_init(&pwm_0, &pwm_0_cfg, NULL);
    sequence_init();    
}

void sequence_init(void)
{
    for (uint16_t i = 0; i < PWM_SEQ_STEPS; i++)
    {
        uint16_t light_power = ((i * NRFX_PWM_DEFAULT_CONFIG_TOP_VALUE) / PWM_SEQ_STEPS);
        pwm_seq_0_values[i].channel_0 = light_power;
        pwm_seq_1_values[PWM_SEQ_STEPS -1 - i].channel_0 = light_power;
    }
       
    pwm_sequence_0.values.p_individual = pwm_seq_0_values;
    pwm_sequence_1.values.p_individual = pwm_seq_1_values;
   
    pwm_sequence_0.length = pwm_sequence_1.length = PWM_SEQ_STEPS;
    pwm_sequence_0.repeats = pwm_sequence_1.repeats = 2;
    pwm_sequence_0.end_delay = pwm_sequence_1.end_delay = 0;
}



void esl_pwm_led_smooth_blinking_mode_device_id(void)
{
    
    uint8_t leds_list[ESL_LEDS_NUMBER] = ESL_LEDS_IDX_LIST;
    uint16_t blinks_num[ESL_LEDS_NUMBER] = ESL_DEVICE_ID;

    for (uint32_t i = 0; i < ESL_LEDS_NUMBER; i++)
    {  
        esl_pwm_init(leds_list[i]);
        esl_pwm_led_smooth_blinking(leds_list[i], blinks_num[i]);
    }
}

*/




