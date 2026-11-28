#ifndef ESL_LOG_H
#define ESL_LOG_H

#include "nrf_log.h"
#include "nrf_log_internal.h"

#define ESL_LOG_ERROR(...)      NRF_LOG_INTERNAL_ERROR(__VA_ARGS__)
#define ESL_LOG_WARNING(...)    NRF_LOG_INTERNAL_WARNING( __VA_ARGS__)
#define ESL_LOG_INFO(...)       NRF_LOG_INTERNAL_INFO( __VA_ARGS__)
#define ESL_LOG_DEBUG(...)      NRF_LOG_INTERNAL_DEBUG( __VA_ARGS__)

#define LOG_TICK 10

void log_init(void);

void esl_log_process(void);

void esl_log_info(char* string);

void esl_log_color(char* mode_name, uint16_t h, uint8_t s, uint8_t v);

#endif