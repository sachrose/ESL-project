#include "nrf_delay.h"
#include "boards.h"
#include "led.h"

void leds_mode_device_id (uint32_t led_idx, uint32_t blinks_num)
{
    for (uint32_t i = 0; i < (blinks_num * 2); i++)
    {
        bsp_board_led_invert(led_idx);
        nrf_delay_ms(500);
    }
}