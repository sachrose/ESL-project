#include "nrf_assert.h"
#include "nrf.h"
#include "esl_led.h"
#include "esl_button.h"
#include "nrfx_systick.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

static const uint8_t esl_led_list[ESL_LEDS_NUMBER] = ESL_LEDS_LIST;
static const uint8_t blinks_num[ESL_LEDS_NUMBER] = ESL_DEVICE_ID;

static nrfx_pwm_t pwm_0 = NRFX_PWM_INSTANCE(0);
static nrf_pwm_values_common_t pwm_seq_0_values[ESL_PWM_LED_STEPS_ARR_SIZE];

static nrf_pwm_sequence_t pwm_seq_0 = 
{
    .values.p_common = pwm_seq_0_values,
    .length = 1,
    .repeats = 0,
    .end_delay = 0
};

blink_context_t blink_context = 
{
    .blinks = ESL_DEVICE_ID,
    .delay_ms = 0,
    .led_idx = 0,
    .count = blinks_num[0],
    .ready_to_count = true
};

smooth_blink_context_t smooth_blink_context = 
{
    .blinks = ESL_DEVICE_ID,
    .led_pins = ESL_LEDS_LIST,
    .systick_state.time = 0,
    .led_idx = 3,
    .count = 0,
    .current_step = 0
};

static void gpio_output_voltage_setup(void)
{
    // Configure UICR_REGOUT0 register only if it is set to default value.
    if ((NRF_UICR->REGOUT0 & UICR_REGOUT0_VOUT_Msk) ==
        (UICR_REGOUT0_VOUT_DEFAULT << UICR_REGOUT0_VOUT_Pos))
    {
        NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Wen;
        while (NRF_NVMC->READY == NVMC_READY_READY_Busy){}

        NRF_UICR->REGOUT0 = (NRF_UICR->REGOUT0 & ~((uint32_t)UICR_REGOUT0_VOUT_Msk)) |
                            (UICR_REGOUT0_VOUT_3V0 << UICR_REGOUT0_VOUT_Pos);

        NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Ren;
        while (NRF_NVMC->READY == NVMC_READY_READY_Busy){}

        // System reset is needed to update UICR registers.
        NVIC_SystemReset();
    }
}

void esl_gpio_leds_init(void)
{
   if (NRF_POWER->MAINREGSTATUS &
    (POWER_MAINREGSTATUS_MAINREGSTATUS_High << POWER_MAINREGSTATUS_MAINREGSTATUS_Pos))
    {
        gpio_output_voltage_setup();
    }
    
    uint32_t i;
    for (i = 0; i < ESL_LEDS_NUMBER; ++i)
    {
        nrf_gpio_cfg_output(esl_led_list[i]);
    }
    esl_gpio_change_all_leds_state(!LEDS_ACTIVE_STATE);
}

bool esl_gpio_led_state_get(uint32_t led_idx)
{
    ASSERT(led_idx < ESL_LEDS_NUMBER);
    bool pin_set = nrf_gpio_pin_out_read(esl_led_list[led_idx]) ? true : false;
    return !pin_set;
}

void esl_gpio_led_invert(uint32_t led_idx)
{
    ASSERT(led_idx < ESL_LEDS_NUMBER);
    nrf_gpio_pin_toggle(esl_led_list[led_idx]);
}

/* Use macro LEDS_ACTIVE_STATE to turn on the led */
void esl_gpio_change_one_led_state(uint32_t led_idx, bool leds_active_state)
{
    ASSERT(led_idx < ESL_LEDS_NUMBER);
    nrf_gpio_pin_write(esl_led_list[led_idx], leds_active_state);
}

/* Use macro LEDS_ACTIVE_STATE to turn on the leds */
void esl_gpio_change_all_leds_state(bool leds_active_state)
{
    uint32_t i;

    for (i = 0; i < ESL_LEDS_NUMBER; ++i)
        {
            esl_gpio_change_one_led_state(i, leds_active_state);
        }
}

void esl_gpio_led_blinking_mode_device_id(void)
{
    for (uint32_t i = 0; i < ESL_LEDS_NUMBER; i++)
    {
        for (uint8_t j = 0; j < (blinks_num[i] * 2); j++)
        {
            esl_gpio_led_invert(i);
            esl_delay_ms(500);
        }
    } 
}

void esl_pwm_led_init(uint8_t led_pin)
{   
    nrfx_pwm_config_t pwm_0_cfg = ESL_PWM_LED_CONFIG(led_pin);
    
    nrfx_pwm_init(&pwm_0, &pwm_0_cfg, NULL); 
}

void esl_pwm_led_seq_values_init(void)
{
    for (uint16_t i = 0; i < ESL_PWM_LED_STEPS_ARR_SIZE; i++)
    {
        uint16_t value = (i <= ESL_PWM_LED_STEPS_ARR_SIZE / 2) ? i : (ESL_PWM_LED_STEPS_ARR_SIZE - 1 - i);
        pwm_seq_0_values[i] = value * ESL_PWM_LED_ONE_STEP_DUTY;
    }
}

void led_blink_iteration(void)
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
        goto led_blink_iteration_end;
    }  

    if (blink_context.count > 0 && blink_context.ready_to_count)
    {
        blink_context.count--;
    }
    
    if (blink_context.count == 0)
    {
        blink_context.led_idx = (blink_context.led_idx + 1) % ESL_LEDS_NUMBER;
        blink_context.count = blinks_num[blink_context.led_idx];
    }

led_blink_iteration_end:
    return;
}

void led_smooth_blink_iteration(void)
{   
    if (smooth_blink_context.count == 0)
    {
        smooth_blink_context.led_idx = (smooth_blink_context.led_idx + 1) % ESL_LEDS_NUMBER;
        smooth_blink_context.count = smooth_blink_context.blinks[smooth_blink_context.led_idx];

        nrfx_pwm_uninit(&pwm_0);
        esl_pwm_led_init(smooth_blink_context.led_pins[smooth_blink_context.led_idx]);
        nrfx_systick_get(&smooth_blink_context.systick_state);
        NRF_LOG_DEBUG("start blinking by LED_%d, count %d", smooth_blink_context.led_idx, smooth_blink_context.count);
    }

    if (smooth_blink_context.count > 0)
    {
        if (nrfx_systick_test(&smooth_blink_context.systick_state, ESL_PWM_LED_DUTY_DURATION_US))
        {
            smooth_blink_context.current_step = (smooth_blink_context.current_step + 1) % ESL_PWM_LED_STEPS_ARR_SIZE;
            pwm_seq_0.values.p_common = &pwm_seq_0_values[smooth_blink_context.current_step];
            
            nrfx_systick_get(&smooth_blink_context.systick_state);
            nrfx_pwm_simple_playback(&pwm_0, &pwm_seq_0, 1, NRFX_PWM_FLAG_LOOP);

            if (smooth_blink_context.current_step == (ESL_PWM_LED_STEPS_ARR_SIZE - 1))
            {
                smooth_blink_context.count--;
            }
        }        
    }
}
