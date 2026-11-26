#include "esl_features.h"
#include "esl_led.h"
#include "esl_button.h"
#include "esl_eeprom.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_backend_usb.h"
#include "nrf_log_default_backends.h"

static const uint16_t esl_device_id[ESL_LEDS_NUMBER] = ESL_DEVICE_ID;

#if ESTC_USB_CLI_ENABLED
static char m_rx_buffer[READ_SIZE];
static char cmd_buffer[CMD_SIZE];
static uint32_t cmd_buf_idx = 0;
bool cmd_ready = false;

static void usb_ev_handler(app_usbd_class_inst_t const * p_inst,
                           app_usbd_cdc_acm_user_event_t event)
{
    switch (event)
    {
    case APP_USBD_CDC_ACM_USER_EVT_PORT_OPEN:
    {
        cmd_buf_idx = 0;
        cmd_ready = false;
        app_usbd_cdc_acm_read(&usb_cdc_acm, m_rx_buffer, READ_SIZE);
        break;
    }
    case APP_USBD_CDC_ACM_USER_EVT_PORT_CLOSE:
    {
        break;
    }
    case APP_USBD_CDC_ACM_USER_EVT_TX_DONE:
    {
        NRF_LOG_INFO("tx done");
        break;
    }
    case APP_USBD_CDC_ACM_USER_EVT_RX_DONE:
    {
        ret_code_t ret;
        do
        {
            size_t size = app_usbd_cdc_acm_rx_size(&usb_cdc_acm);
            NRF_LOG_INFO("rx size: %d", size);

            if (m_rx_buffer[0] == '\r' || m_rx_buffer[0] == '\n')
            {
                if (cmd_buf_idx > 0)
                {
                    cmd_buffer[cmd_buf_idx] = '\0';
                    cmd_buf_idx = 0;
                    cmd_ready = true;
                }
                ret = app_usbd_cdc_acm_write(&usb_cdc_acm, "\r\n", 2);
            }
            else if (m_rx_buffer[0] == '\b')
            {
                if (cmd_buf_idx > 0)
                {
                    cmd_buf_idx--;
                    ret = app_usbd_cdc_acm_write(&usb_cdc_acm, "\b \b", 3);
                }
            }
            else if (!cmd_ready && cmd_buf_idx < (CMD_SIZE - 1))
            {
                cmd_buffer[cmd_buf_idx++] = m_rx_buffer[0];
                ret = app_usbd_cdc_acm_write(&usb_cdc_acm, m_rx_buffer, READ_SIZE);
            }

            ret = app_usbd_cdc_acm_read(&usb_cdc_acm, m_rx_buffer, READ_SIZE);
        } while (ret == NRF_SUCCESS);
    }
   
    default:
        break;
    }
}

APP_USBD_CDC_ACM_GLOBAL_DEF(usb_cdc_acm,
                            usb_ev_handler,
                            CDC_ACM_COMM_INTERFACE,
                            CDC_ACM_DATA_INTERFACE,
                            CDC_ACM_COMM_EPIN,
                            CDC_ACM_DATA_EPIN,
                            CDC_ACM_DATA_EPOUT,
                            APP_USBD_CDC_COMM_PROTOCOL_NONE);

static void cli_init(void)
{
    app_usbd_class_inst_t const * class_cdc_acm = app_usbd_cdc_acm_class_inst_get(&usb_cdc_acm);
    app_usbd_class_append(class_cdc_acm);
}

static void cli_process(char *cmd)
{

}
#endif

static void log_init(void)
{
    ret_code_t ret = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(ret);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}

void hardware_init(void)
{
    log_init();
    NRF_LOG_INFO(">hardware_init()");

#if ESTC_USB_CLI_ENABLED
       cli_init(void)
#endif

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
    state_read_handler(&start_color);
    esl_pwm_led_config(start_color, state_write_handler);
    NRF_LOG_DEBUG("Init PWM - End");

    NRF_LOG_INFO("<hardware_init()");
}

void log_process(void)
{
    LOG_BACKEND_USB_PROCESS();
    NRF_LOG_PROCESS();
}