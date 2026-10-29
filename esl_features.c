#include "esl_features.h"
#include "esl_button.h"

static blink_context_t blink_context = 
{
    .how_much_is_the_fish = {6, 6, 1, 3},
    .led_idx = LED_IDX_YELLOW_GREEN,
    .count = 0
};

void blink_iteration(void)
{
    for (size_t i = 0; i < 2; i++)
    {
        esl_led_invert(blink_context.led_idx);
        esl_delay_ms(500);
    }
    
    if (blink_context.count > 0)
    {
        blink_context.count--;
    }

    if (blink_context.count == 0)
    {
        blink_context.led_idx = (blink_context.led_idx + 1) % ESL_LEDS_NUMBER;
        blink_context.count = blink_context.how_much_is_the_fish[blink_context.led_idx];
    }  
}

void led_blinking_mode_button(void)
{
    if (esl_button_state_get())
    {
        blink_iteration();
    }  
}