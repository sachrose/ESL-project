#ifndef ESL_FEATURES_H
#define ESL_FEATURES_H

#include "app_usbd.h"
#include "app_usbd_serial_num.h"
#include "app_usbd_cdc_acm.h"
#include "nordic_common.h"

// === USB CDC ACM для CLI — ОТЛИЧНЫЕ от логов номера ===
// Логи уже занимают: COMM_IF=0, DATA_IF=1, COMM_EPIN=2, DATA_EPIN=1, DATA_EPOUT=1
// CLI должен занять следующее свободное:
/*
#define NRF_CLI_USB_CDC_ACM_COMM_INTERFACE 2
#define NRF_CLI_USB_CDC_ACM_DATA_INTERFACE 3
#define NRF_CLI_USB_CDC_ACM_COMM_EPIN      3
#define NRF_CLI_USB_CDC_ACM_DATA_EPIN      4
#define NRF_CLI_USB_CDC_ACM_DATA_EPOUT     2
*/

#define CDC_ACM_COMM_INTERFACE  2
#define CDC_ACM_COMM_EPIN       NRF_DRV_USBD_EPIN3
#define CDC_ACM_DATA_INTERFACE  3
#define CDC_ACM_DATA_EPIN       NRF_DRV_USBD_EPIN4
#define CDC_ACM_DATA_EPOUT      NRF_DRV_USBD_EPOUT4

#define READ_SIZE   1
#define CMD_SIZE    16

void hardware_init(void);

void log_process(void);

#endif