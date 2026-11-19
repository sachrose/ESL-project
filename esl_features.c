#include "esl_features.h"
#include "esl_led.h"
#include "esl_button.h"
#include "esl_eeprom.h"
#include "app_timer.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

static const uint16_t esl_device_id[ESL_LEDS_NUMBER] = ESL_DEVICE_ID;

void hardware_init(void)
{
    logs_init();
    NRF_LOG_INFO(">hardware_init()");
    app_timer_init();
    NRF_LOG_DEBUG("Init the LF clock for USB, init app timer - End");

    esl_button_config(change_mode, start_editing, stop_editing);
    NRF_LOG_DEBUG("Init button - End");
 
    hsv_word_t start_color =
    {
        .hue = ESL_DEFAULT_HUE(esl_device_id[2], esl_device_id[3]),
        .sat = ESL_DEFAULT_SATURATION,
        .val = ESL_DEFAULT_VALUE
    };
    app_data_load(&start_color);
    esl_pwm_led_config(start_color, app_data_save);
    NRF_LOG_DEBUG("Init PWM - End");

    NRF_LOG_INFO("<hardware_init()");
}

void logs_init(void)
{
    ret_code_t ret = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(ret);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}