#include "nrf_assert.h"
#include "nrf_gpio.h"
#include "nrf.h"
#include "esl_led.h"

static const uint8_t esl_led_list[ESL_LEDS_NUMBER] = ESL_LEDS_LIST;

static void gpio_output_voltage_setup(void)
{
    // Configure UICR_REGOUT0 register only if it is set to default value.
    if ((NRF_UICR->REGOUT0 & UICR_REGOUT0_VOUT_Msk) ==
        (UICR_REGOUT0_VOUT_DEFAULT << UICR_REGOUT0_VOUT_Pos))
    {
        NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Wen;
        while (NRF_NVMC->READY == NVMC_READY_READY_Busy){}

        NRF_UICR->REGOUT0 = (NRF_UICR->REGOUT0 & ~((uint32_t)UICR_REGOUT0_VOUT_Msk)) |
                            (UICR_REGOUT0_VOUT_3V0 << UICR_REGOUT0_VOUT_Pos);

        NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Ren;
        while (NRF_NVMC->READY == NVMC_READY_READY_Busy){}

        // System reset is needed to update UICR registers.
        NVIC_SystemReset();
    }
}

void esl_leds_init(void)
{
   if (NRF_POWER->MAINREGSTATUS &
    (POWER_MAINREGSTATUS_MAINREGSTATUS_High << POWER_MAINREGSTATUS_MAINREGSTATUS_Pos))
    {
        gpio_output_voltage_setup();
    }
    
    uint32_t i;
    for (i = 0; i < ESL_LEDS_NUMBER; ++i)
    {
        nrf_gpio_cfg_output(esl_led_list[i]);
    }
    esl_leds_off();
}

void esl_led_invert(uint32_t led_idx)
{
    ASSERT(led_idx < ESL_LEDS_NUMBER);
    nrf_gpio_pin_toggle(esl_led_list[led_idx]);
}

void esl_led_on(uint32_t led_idx)
{
    ASSERT(led_idx < ESL_LEDS_NUMBER);
    nrf_gpio_pin_write(esl_led_list[led_idx], LEDS_ACTIVE_STATE ? 1 : 0);
}

void esl_led_off(uint32_t led_idx)
{
    ASSERT(led_idx < ESL_LEDS_NUMBER);
    nrf_gpio_pin_write(esl_led_list[led_idx], LEDS_ACTIVE_STATE ? 0 : 1);
}

void esl_leds_off(void)
{
    uint32_t i;
    for (i = 0; i < ESL_LEDS_NUMBER; ++i)
    {
        esl_led_off(i);
    }
}

void esl_leds_on(void)
{
    uint32_t i;
    for (i = 0; i < ESL_LEDS_NUMBER; ++i)
    {
        esl_led_on(i);
    }
}

void led_blinking_mode_device_id (void)
{
    static uint32_t led_idx = 0;
    uint32_t blinks_num;

    switch (led_idx)
    {
        case LED_IDX_YELLOW_GREEN:
        case LED_IDX_RED:
            blinks_num = 6;
            break;
        case LED_IDX_GREEN:
            blinks_num = 1;
            break;
        case LED_IDX_BLUE:
            blinks_num = 3;
            break;

        /*This code will be executed if something goes wrong.*/
        default:
            while (true)
            {
                esl_led_invert(1);
                esl_delay_ms(200);
            }
    }

    led_blinking (led_idx, blinks_num);
    led_idx = (led_idx + 1) % ESL_LEDS_NUMBER;       
}

void led_blinking (uint32_t led_idx, uint32_t blinks_num)
{
    for (uint32_t i = 0; i < (blinks_num * 2); i++)
    {
        esl_led_invert(led_idx);
        esl_delay_ms(500);
    }
}