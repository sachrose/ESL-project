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
APP_USBD_CDC_ACM_GLOBAL_DEF(usb_cdc_acm,
                            usb_ev_handler,
                            CDC_ACM_COMM_INTERFACE,
                            CDC_ACM_DATA_INTERFACE,
                            CDC_ACM_COMM_EPIN,
                            CDC_ACM_DATA_EPIN,
                            CDC_ACM_DATA_EPOUT,
                            APP_USBD_CDC_COMM_PROTOCOL_NONE);

static cli_context_t cli_context
{
    .state = SLEEP,
    .cmd_buf_idx = 0,
    .cmd_buf_overflow = false,
    .cmd_ready = false
};

static const cli_command_t commands_list[] = 
{
    {"RGB", "RGB <red> <green> <blue> - the device sets current color to specified one.", cmd_rgb_handler},
    {"HSV", "HSV <hue> <saturation> <value> - the same with RGB, but color is specified in HSV.", cmd_hsv_handler},
    {"help", "help - print information about supported commands.", cmd_help_handler}
}

static const char* cli_errors_list[CLI_ERROR_COUNT] =
{
    [UNKNOWN] = "Error: Unknown command!\r\n",
    [RANGE_255] = "Error: R, G, B must be numbers in range 0..255\r\n",
    [RANGE_360] ="Error: R, G, B must be numbers in range 0..360\r\n",
    [OVERFLOW_BUF] = "Error: Buffer overflow!\r\n"
};

static void cli_init(void)
{
    app_usbd_class_inst_t const * class_cdc_acm = app_usbd_cdc_acm_class_inst_get(&usb_cdc_acm);
    app_usbd_class_append(class_cdc_acm);
}

static void esl_cli_print(char* string)
{
    uint32_t length = strlen(string);
    if (length == 0)
    {
        return;
    }

    while (app_usbd_cdc_acm_write(&usb_cdc_acm, string, length) == NRF_ERROR_BUSY)
    {
        while (app_usbd_event_queue_process())
        {
            /* Nothing to do */
        }
    }
}

static bool is_valid_number(const char* string, uint32_t min, uint32_t max, uint32_t* out_value)
{
    bool result = false;
    uint32_t value = 0;

    if (string == NULL || out_value == NULL || string[0] == '\0' || min > max)
    {
        goto is_valid_end;
    }
    
    for (uint32_t i = 0; string[i] != '\0'; i++)
    {
        if (string[i] < '0' || string[i] > '9')
        {
            goto is_valid_end;
        }
        
        value = value * 10 + (uint32_t)(string[i] - '0');

        if (value > max )
        {
            goto is_valid_end;
        }      
    }

    if (value < min)
    {
        goto is_valid_end;
    }
    
    *out_value = value;
    result = true;

is_valid_end:
    return result;
}

static void usb_ev_handler(app_usbd_class_inst_t const * p_inst,
                           app_usbd_cdc_acm_user_event_t event)
{
    switch (event)
    {
        case APP_USBD_CDC_ACM_USER_EVT_PORT_OPEN:
        {
            esl_fsm_cli(PORT_OPEN);
            break;
        }
        case APP_USBD_CDC_ACM_USER_EVT_PORT_CLOSE:
        {
            esl_fsm_cli(PORT_CLOSE);
            break;
        }
        case APP_USBD_CDC_ACM_USER_EVT_TX_DONE:
        {
            NRF_LOG_INFO("TX done");
            break;
        }
        case APP_USBD_CDC_ACM_USER_EVT_RX_DONE:
            ret_code_t ret;
            do
            {   
                size_t size = app_usbd_cdc_acm_rx_size(&usb_cdc_acm);
                if (size > 0)
                {
                    esl_fsm_cli(RX_CHAR);
                }
                ret = app_usbd_cdc_acm_read(&usb_cdc_acm, cli_context.rx_byte, READ_SIZE);
            } while (ret == NRF_SUCCESS);
        
        default:
            break;
    }
}

static void cli_execute_command(char* command)
{
    uint32_t count = 0;
    char *all_args[ARGS_COUNT] = {NULL};
    char *arg = strtok(command, " ");
        
    while (arg != NULL && count < ARGS_COUNT)
    {
        all_args[count++] = arg;
        arg = strtok(NULL, " ");
    }

    if (count == 0)
    {
        goto cli_execute_command_end;
    }

    for (uint32_t i = 0; i < CMD_COUNT; i++)
    {
        if (strcmp(all_args[0], commands_list[i].name) == 0)
        {
            commands_list[i].handler(all_args, count);
            goto cli_execute_command_end;
        }
    }

    cli_print(cli_errors_list[UNKNOWN]);

cli_execute_command_end:
    return;
}

static void cmd_rgb_handler(char** command, int count)
{
    if (count != 4)
    {
        cli_print(cli_errors_list[UNKNOWN]);
        goto cmd_rgb_end;
    }

    uint32_t r, g, b;
    if (is_valid_number(command[1], MIN_COLOR_VALUE, MAX_RGB, &r) &&
        is_valid_number(command[2], MIN_COLOR_VALUE, MAX_RGB, &g) &&
        is_valid_number(command[3], MIN_COLOR_VALUE, MAX_RGB, &b))
    {
        rgb_from_cli((uint8_t)r, (uint8_t)g, (uint8_t)b);
    }
    else
    {
        cli_print(cli_errors_list[RANGE_255]);
    }

cmd_rgb_end:
    return;
}

static void cmd_hsv_handler(char** command, int count)
{
    if (count != 4)
    {
        cli_print(cli_errors_list[UNKNOWN]);
        goto cmd_hsv_end;
    }

    uint32_t h, s, v;
    if (is_valid_number(argv, MIN_COLOR_VALUE, MAX_HUE, &h) &&
        is_valid_number(argv, MIN_COLOR_VALUE, MAX_SAT, &s) &&
        is_valid_number(argv, MIN_COLOR_VALUE, MAX_VAL, &v))
    {
        hsv_from_cli((uint16_t)h, (uint8_t)s, (uint8_t)v);        
    }
    else
    {
        cli_print(cli_errors_list[RANGE_360]);
    } 

cmd_hsv_end:
    return;
}

static void cmd_help_handler(char** command, int count)
{
    if (count != 1)
    {
        cli_print(cli_errors_list[UNKNOWN]);
        goto cmd_help_end;
    }

    cli_print("\t\tSupported commands:\r\n");
    for (uint32_t i = 0; i < CMD_COUNT; i++)
    {
        cli_print("\t");
        cli_print(commands_list[i].description);
        cli_print("\r\n");
    }

cmd_help_end:
    return;
}

static esl_fsm_cli(cli_event_t event)
{
    switch (cli_context.state)
    {
        case SLEEP:
        {
            if (event == PORT_OPEN)
            {
                cli_context.cmd_buf_idx = 0;
                cli_context.cmd_ready = false;
                cli_context.state = RECEIVING;
                app_usbd_cdc_acm_read(&usb_cdc_acm, cli_context.rx_byte, READ_SIZE);
            }
            break;
        }
        case RECEIVING:
        {
            if (event == PORT_CLOSE)
            {
                cli_context.state = SLEEP;
            }
            else if (event == RX_CHAR)
            {
                char ch = *cli_context.rx_byte;

                if (cli_context.cmd_buf_overflow)
                {
                    cli_context.cmd_buf_idx = 0;
                    cli_context.cmd_buf_overflow = false;
                    cli_print(cli_errors_list[OVERFLOW_BUF]);
                }
                
                if (ch == '\r' || ch == '\n')
                {
                    esl_cli_print("\r\n");
                    if (cli_context.cmd_buf_idx > 0)
                    {
                        cli_context.cmd_buffer[cli_context.cmd_buf_idx] = '\0';
                        cli_context.cmd_buf_idx = 0;
                        cli_context.cmd_ready = true;
                        cli_context.state = EXECUTION;
                    }
                }
                else if (ch == '\b')
                {
                    if (cli_context.cmd_buf_idx > 0)
                    {
                        cli_context.cmd_buf_idx--;
                        cli_context.cmd_buf_overflow = false;
                        esl_cli_print("\b \b");
                    }
                }
                else 
                    if (!cli_context.cmd_ready && cli_context.cmd_buf_idx < (CMD_SIZE - 1))
                    {
                        cli_context.cmd_buffer[cli_context.cmd_buf_idx++] = ch;
                        esl_cli_print(cli_context.rx_byte);
                    }
                    else
                    {
                        cli_context.cmd_buf_overflow = true;
                    }
            break;
            }
        }
        case EXECUTION:
        {
            if (event ==  PORT_CLOSE)
            {
                cli_context.cmd_ready = false;
                cli_context.state = SLEEP;
            }
            
            if (event ==  JUST_DO_IT)
            {
                cli_execute_command(cli_context.cmd_buffer);
                cli_context.cmd_ready = false;
                cli_context.state = RECEIVING;
            }                                            
            break;
        }    
        default:
        {
            break;
        }
    }
}

void esl_cli_process(char *cmd)
{
    if (cli_context.cmd_ready)
    {
        esl_fsm_cli(JUST_DO_IT);
    }   
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

    esl_button_config(view_current_color, change_mode, start_editing, stop_editing);
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

void esl_log_process(void)
{
    LOG_BACKEND_USB_PROCESS();
    NRF_LOG_PROCESS();
}