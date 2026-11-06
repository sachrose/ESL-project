#include "esl_features.h"
#include "esl_button.h"
#include "esl_led.h"
#include "nrfx_gpiote.h"
#include "nrfx_pwm.h"
#include "nrf_drv_clock.h"
#include "nrf_log.h"

static const nrfx_pwm_t pwm = NRFX_PWM_INSTANCE(0);

static nrfx_pwm_config_t pwm_cfg = NRFX_PWM_DEFAULT_CONFIG;

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
    ret_code_t err_code;

    err_code = nrf_drv_clock_init();
    APP_ERROR_CHECK(err_code);
    nrf_drv_clock_lfclk_request(NULL);
    while (!nrf_drv_clock_lfclk_is_running())
    {
        /* Just waiting until the lf clock starts up */
    }
    NRF_LOG_DEBUG("Init the LF clock for USB - End");

    err_code = nrfx_gpiote_init();
    APP_ERROR_CHECK(err_code);

    nrfx_gpiote_in_config_t context = NRFX_GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);
    context.pull = NRF_GPIO_PIN_PULLUP; 
    err_code = nrfx_gpiote_in_init(BUTTON, &context, NULL);
    APP_ERROR_CHECK(err_code);

    nrfx_gpiote_in_event_enable(BUTTON, true);
    NRF_LOG_DEBUG("Init GPIOTE - End");

    pwm_cfg.output_pins[0] = LED1_G;
    pwm_cfg.output_pins[1] = LED2_R;
    pwm_cfg.output_pins[2] = LED2_G;
    pwm_cfg.output_pins[3] = LED2_B;
    err_code = nrfx_pwm_init(&pwm, &pwm_cfg, NULL);
    APP_ERROR_CHECK(err_code);
    NRF_LOG_DEBUG("Init PWM - End");
        
    esl_leds_init();
    esl_button_init();
}

void blink_iteration(void)
{
    if (blink_context.delay_ms == 0)    // после обычного завершения функции
    {
        esl_led_invert(blink_context.led_idx);
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