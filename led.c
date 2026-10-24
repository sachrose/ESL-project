#include "nrf_delay.h"
#include "boards.h"
#include "led.h"

void led_blinking_mode_device_id (void)
{
    static uint32_t led_idx = 0;
    uint32_t blinks_num;

    switch (led_idx)
    {
        case LED_YELLOW_GREEN:
        case LED_RED:
            blinks_num = 6;
            break;
        case LED_GREEN:
            blinks_num = 1;
            break;
        case LED_BLUE:
            blinks_num = 3;
            break;

        /*This code will be executed if something goes wrong.*/
        default:
            while (true)
            {
                bsp_board_led_invert(1);
                nrf_delay_ms(200);
            }
    }

    led_blinking (led_idx, blinks_num);
    led_idx = (led_idx + 1) % LEDS_NUMBER;       
}

void led_blinking (uint32_t led_idx, uint32_t blinks_num)
{
    for (uint32_t i = 0; i < (blinks_num * 2); i++)
    {
        bsp_board_led_invert(led_idx);
        nrf_delay_ms(500);
    }
}