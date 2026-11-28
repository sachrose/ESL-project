#include "esl_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_backend_usb.h"
#include "nrf_log_default_backends.h"

void log_init(void)
{
    ret_code_t ret = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(ret);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}

void esl_log_process(void)
{
    LOG_BACKEND_USB_PROCESS();
    NRF_LOG_PROCESS();
}

void esl_log_info(char* string)
{
    ESL_LOG_INFO("%s", string);
}

void esl_log_color(char* mode_name, uint16_t h, uint8_t s, uint8_t v)
{
    ESL_LOG_INFO("%s H = %d, S = %d, V = %d", mode_name, h, s, v);
}