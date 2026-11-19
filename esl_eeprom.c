#include <stdint.h>
#include "esl_eeprom.h"
#include "nrfx_nvmc.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

static uint32_t addr_to_write = APP_DATA_START_ADDR;
static hsv_word_t* last_word = NULL;

static void app_data_erase(void)
{
    for (uint32_t i = 0; i < APP_DATA_PAGE_COUNT; i++)
    {
        uint32_t page_addr = (APP_DATA_START_ADDR + (i * APP_DATA_PAGE_SIZE));
        nrfx_nvmc_page_erase(page_addr);
    }
    addr_to_write = APP_DATA_START_ADDR;
}

static bool is_word_empty(hsv_word_t* word)
{
    return (word->hue == UINT16_MAX && word->sat == UINT8_MAX && word->val == UINT8_MAX);
}

static bool is_word_valid(hsv_word_t* word)
{
    return (word->hue <= MAX_HUE && word->sat <= MAX_SAT && word->val <= MAX_VAL);
}

void app_data_load(hsv_word_t* hsv_word)
{
    for (uint32_t addr = APP_DATA_START_ADDR; addr < APP_DATA_END_ADDR; addr += sizeof(hsv_word_t))
    {
        hsv_word_t* current_addr = (hsv_word_t*)addr;

        if (is_word_empty(current_addr))
        {
            addr_to_write = addr;
        }
        else if (is_word_valid(current_addr))
        {
            last_word = current_addr;
        }
        else
        {
            NRF_LOG_INFO("There is no empty space. Need to erase all app_data!");
            app_data_erase();
        }
    }

    if (last_word != NULL)
    {
        *hsv_word = *last_word;
        NRF_LOG_INFO("HUE was loaded from EEPROM");
    }
}

void app_data_save(hsv_word_t new_hsv_word)
{   
    uint32_t word = (((uint32_t)new_hsv_word.val << 24)| ((uint32_t)new_hsv_word.sat << 16) | ((uint32_t)new_hsv_word.hue));

    if (nrfx_nvmc_word_writable_check(addr_to_write, word))
    {
        nrfx_nvmc_word_write(addr_to_write, word);
        while (!nrfx_nvmc_write_done_check()) {}
    }

    addr_to_write += sizeof(hsv_word_t);
}