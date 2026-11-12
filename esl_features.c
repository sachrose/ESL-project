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

void hardware_smooth_init(void)
{
    logs_init();
    NRF_LOG_INFO(">hardware_init()");
    NRF_LOG_DEBUG("Init the LF clock for USB - End");
   
    app_timer_init();
    apptimer2_init_for_button_handlers();
    NRF_LOG_DEBUG("Init the APP_TIMER2 for button IRQhandlers - End");

    nrfx_gpiote_init();
    esl_button_gpiote_config();
    NRF_LOG_DEBUG("Init GPIOTE - End"); 

    esl_pwm_led_init();
    NRF_LOG_DEBUG("Init PWM - End");
    NRF_LOG_INFO("<hardware_init()");
}

void logs_init(void)
{
    ret_code_t ret = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(ret);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}