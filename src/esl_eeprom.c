#include <stdint.h>
#include <string.h>
#include "esl_log.h"
#include "esl_eeprom.h"
#include "nrfx_nvmc.h"

static uint32_t addr_to_write = APP_DATA_START_ADDR;
static hsv_word_t* last_word = NULL;

static bool is_word_empty(hsv_word_t* word)
{
    return (word->hue == UINT16_MAX && word->sat == UINT8_MAX && word->val == UINT8_MAX);
}

static bool is_word_valid(hsv_word_t* word)
{
    return (word->hue <= MAX_HUE && word->sat <= MAX_SAT && word->val <= MAX_VAL);
}

static void state_erase_handler(void)
{
    esl_log_info("There is no empty space. Need to erase all app_data!");

    for (uint32_t i = 0; i < APP_DATA_PAGE_COUNT; i++)
    {
        uint32_t page_addr = (APP_DATA_START_ADDR + (i * APP_DATA_PAGE_SIZE));
        nrfx_nvmc_page_erase(page_addr);
    }
    addr_to_write = APP_DATA_START_ADDR;
    last_word = NULL;
}

void state_read_handler(hsv_word_t* hsv_word)
{    
    for (uint32_t addr = APP_DATA_START_ADDR; addr < APP_DATA_END_ADDR; addr += sizeof(hsv_word_t))
    {        
        hsv_word_t* current_addr = (hsv_word_t*)addr;

        if (is_word_empty(current_addr))
        {
            addr_to_write = addr;
            break;
        }
        else if (is_word_valid(current_addr))
        {
            last_word = current_addr;
            continue;
        }
        else
        {
            state_erase_handler();
            break;
        }
    }

    if (last_word != NULL)
    {
        *hsv_word = *last_word;
        esl_log_info("HUE was loaded from EEPROM");
    }
}

void state_write_handler(hsv_word_t hsv_word)
{
    uint32_t word_to_write;
    memcpy(&word_to_write, &hsv_word, sizeof(word_to_write));

    if (nrfx_nvmc_word_writable_check(addr_to_write, word_to_write))
    {
        nrfx_nvmc_word_write(addr_to_write, word_to_write);
        while (!nrfx_nvmc_write_done_check()) {}
    }

    addr_to_write += sizeof(hsv_word_t);

    if (addr_to_write >= APP_DATA_END_ADDR)
    {
        state_erase_handler();
    }
}