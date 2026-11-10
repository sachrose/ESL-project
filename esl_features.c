#include "esl_features.h"
#include "esl_button.h"
#include "esl_led.h"
#include "nrfx_gpiote.h"
#include "nrf_gpiote.h"
#include "nrf_drv_clock.h"
#include "app_timer.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

extern blink_context_t blink_context;
extern smooth_blink_context_t smooth_blink_context;
extern button_context_t button_context;

void hardware_init()
{
    logs_init();
    NRF_LOG_INFO(">hardware_init()");
    nrfx_systick_init();
    nrfx_systick_get(&smooth_blink_context.systick_state);
    NRF_LOG_DEBUG("Init the LF clock for USB - End. Init the systick for PWM - End");
   
    app_timer_init();
    apptimer2_init_for_button_handlers();
    NRF_LOG_DEBUG("Init the APP_TIMER2 for button IRQhandlers- End");

    nrfx_gpiote_init();
    esl_button_gpiote_config();
    nrfx_gpiote_in_event_enable(BUTTON, true);
    NRF_LOG_DEBUG("Init GPIOTE - End"); 

    esl_pwm_led_seq_values_init();
    //esl_pwm_led_init(LED2_B);
    NRF_LOG_DEBUG("Init PWM - End");
    NRF_LOG_INFO("<hardware_init()");
}

void logs_init(void)
{
    ret_code_t ret = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(ret);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}

void led_mode_by_button(led_mode_t mode)
{
    if (button_context.button_button)
    {
        switch (mode)
        {
        case BLINK_MODE_SIMPLE:
            led_blink_iteration();
            break;
        case BLINK_MODE_SMOOTH:
            led_smooth_blink_iteration();
            break;
        }
    }
}