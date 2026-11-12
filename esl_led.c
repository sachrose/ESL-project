#include "nrf_assert.h"
#include "nrf.h"
#include "esl_led.h"
#include "esl_button.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

static const uint8_t esl_led_list[ESL_LEDS_NUMBER] = ESL_LEDS_LIST;
//static const uint8_t blinks_num[ESL_LEDS_NUMBER] = ESL_DEVICE_ID;

static nrfx_pwm_t pwm_0 = NRFX_PWM_INSTANCE(0);
static nrfx_pwm_config_t pwm_0_cfg = NRFX_PWM_DEFAULT_CONFIG;
static nrf_pwm_sequence_t seq;
static nrf_pwm_values_individual_t seq_values[1];
static hsv_context_t hsv_context;

void esl_pwm_led_init(void)
{
    hsv_context.hue = ESL_DEFAULT_HUE(esl_led_list[2], esl_led_list[3]);
    hsv_context.sat = ESL_DEFAULT_SATURATION;
    hsv_context.val = ESL_DEFAULT_VALUE;
    
    
    pwm_0_cfg.load_mode = NRF_PWM_LOAD_INDIVIDUAL;
    for (uint8_t i = 0; i < ESL_LEDS_NUMBER; i++)
    {
        pwm_0_cfg.output_pins[i] = esl_led_list[i] | NRFX_PWM_PIN_INVERTED;
    }

    nrfx_pwm_init(&pwm_0, &pwm_0_cfg, NULL);

    seq_values->channel_0 = 0;
    seq_values->channel_1 = 783;
    seq_values->channel_2 = 1000;
    seq_values->channel_3 = 0;

    seq.values.p_individual = seq_values;
    seq.length = NRF_PWM_VALUES_LENGTH(seq_values);
    //seq.repeats = 0;
    //seq.end_delay = 0;

    nrfx_pwm_simple_playback(&pwm_0, &seq, 1, NRFX_PWM_FLAG_LOOP);
}
/*
    duty cycle  = value from seq / NRFX_PWM_DEFAULT_CONFIG_TOP_VALUE;


    sequence   ->   value = RGB × TOP
    channel   -> duty = value / TOP
*/
