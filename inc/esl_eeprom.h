#ifndef ESL_EEPROM_H
#define ESL_EEPROM_H

#include "esl_types.h"

#define BOOTLOADER_START_ADDR   0xE0000
#define APP_DATA_PAGE_COUNT     3
#define APP_DATA_PAGE_WORDS     1024
#define APP_DATA_PAGE_SIZE      (APP_DATA_PAGE_WORDS * sizeof(uint32_t))            // 4096
#define APP_DATA_AREA_SIZE      ((APP_DATA_PAGE_SIZE) * (APP_DATA_PAGE_COUNT))      // 12288 (0x3000)
#define APP_DATA_START_ADDR     ((BOOTLOADER_START_ADDR) - (APP_DATA_AREA_SIZE))    // 0xDD000
#define APP_DATA_END_ADDR       ((APP_DATA_START_ADDR) + (APP_DATA_AREA_SIZE))      // 0xE0000 (BOOTLOADER_START_ADDR)
#define APP_DATA_EMPTY_WORD     0xFFFFFFFF

void state_read_handler(hsv_word_t* hsv_word);

void state_write_handler(hsv_word_t hsv_word);

#endif