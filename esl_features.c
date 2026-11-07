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

#define PWM_SEQ_STEPS   50

//APP_TIMER_DEF(debounce);
//APP_TIMER_DEF(double_click);

static nrfx_pwm_t pwm_0 = NRFX_PWM_INSTANCE(0);

static nrf_pwm_sequence_t pwm_sequence_0;

static nrf_pwm_sequence_t pwm_sequence_1;

static nrf_pwm_values_individual_t pwm_seq_0_values[PWM_SEQ_STEPS];

static nrf_pwm_values_individual_t pwm_seq_1_values[PWM_SEQ_STEPS];

static blink_context_t blink_context = 
{
    .blinks = {6, 6, 1, 3},
    .delay_ms = 0,
    .led_idx = IDX_LED1_G,
    .count = 6,
    .ready_to_count = true
};

void hardware_init()
{
    
    nrf_drv_clock_init();
    nrf_drv_clock_lfclk_request(NULL);
    while (!nrf_drv_clock_lfclk_is_running()) {}
    NRF_LOG_DEBUG("Init the LF clock for USB - End");
 /*      
    app_timer_init();   
    app_timer_create(&debounce, APP_TIMER_MODE_SINGLE_SHOT, debounce_IRQHandler);
    app_timer_create(&double_click, APP_TIMER_MODE_SINGLE_SHOT, double_click_IRQHandler);
    NRF_LOG_DEBUG("Init the APP_TIMER2 - End");

    nrfx_gpiote_init();
    esl_button_gpiote_config();
    nrfx_gpiote_in_event_enable(BUTTON, true);
    NRF_LOG_DEBUG("Init GPIOTE - End"); 
    */

    //esl_pwm_init(LED1_G);
    //NRF_LOG_DEBUG("Init PWM - End");
}

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

void esl_pwm_led_smooth_blinking(uint8_t led_idx, uint16_t blinks_num)
{
    for (uint16_t i = 0; i < blinks_num; i++)
    {
        nrfx_pwm_complex_playback(&pwm_0, &pwm_sequence_0, &pwm_sequence_1, i, NRFX_PWM_FLAG_STOP);
        
        nrfx_pwm_uninit(&pwm_0);
    }
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


void esl_button_gpiote_config(void)
{  
    nrfx_gpiote_in_config_t context = NRFX_GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);
    context.pull = BUTTON_PULL;

    nrfx_gpiote_in_init(BUTTON, &context, button_IRQHandler);
}

void debounce_IRQHandler(void * p_context)
{
    /*
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
*/
}

void double_click_IRQHandler(void * p_context)
{

/*
    blink_context.button_pressed = false;
    blink_context.clicks_num = 0;
    */
}

void button_IRQHandler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    /*
    if (!blink_context.in_bouncing)
    {
        app_timer_start(debounce, APP_TIMER_TICKS(50), NULL);
        blink_context.in_bouncing = true;
    }
    */
}

void blink_iteration(void)
{
    if (blink_context.delay_ms == 0)
    {
        esl_gpio_led_invert(blink_context.led_idx);
        blink_context.ready_to_count = !blink_context.ready_to_count;
        blink_context.delay_ms = 500;
    }
    
    while (blink_context.delay_ms > 0 && esl_button_state_get())    
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
    if (esl_button_state_get())
    {
        blink_iteration();
    }  
} 
