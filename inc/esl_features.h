#ifndef ESL_FEATURES_H
#define ESL_FEATURES_H

#if ESTC_USB_CLI_ENABLED
#include "app_usbd.h"
#include "app_usbd_serial_num.h"
#include "app_usbd_cdc_acm.h"
#include "nordic_common.h"

#define CDC_ACM_COMM_INTERFACE  2
#define CDC_ACM_COMM_EPIN       NRF_DRV_USBD_EPIN3
#define CDC_ACM_DATA_INTERFACE  3
#define CDC_ACM_DATA_EPIN       NRF_DRV_USBD_EPIN4
#define CDC_ACM_DATA_EPOUT      NRF_DRV_USBD_EPOUT4

#define READ_SIZE   1
#define CMD_SIZE    16

#define CMD_COUNT   3
#define ARGS_COUNT  4

typedef enum
{
    SLEEP = 0,
    RECEIVING = 1,
    EXECUTION = 2,
    CLI_STATE_COUNT = 3
} cli_state_t;

typedef enum
{
    PORT_OPEN = 0,
    PORT_CLOSE = 1,
    RX_CHAR = 2,
    JUST_DO_IT = 3
} cli_event_t;

typedef enum
{
    UNKNOWN = 0,
    RANGE_255,
    RANGE_360,
    OVERFLOW_BUF,
    CLI_ERROR_COUNT
} cli_error_t;

typedef struct
{
    cli_state_t state;
    char rx_byte[READ_SIZE];
    char cmd_buffer[CMD_SIZE];
    uint32_t cmd_buf_idx;
    bool cmd_buf_overflow;
    bool cmd_ready;
} cli_context_t;

typedef struct
{
    char* command_name;
    char* command_description;
    void (*handler)(char** command, uint32_t count);
} cli_command_t;

void esl_cli_process(char* command);
#endif

void hardware_init(void);

void esl_log_process(void);

#endif