#include "esl_features.h"
#include "esl_button.h"

static blink_context_t blink_context = 
{
    .how_much_is_the_fish = {6, 6, 1, 3},
    .delay_ms = 0,
    .led_idx = LED_IDX_YELLOW_GREEN,
    .count = 0,
    .state = READY_TO_ON    
};

void blink_iteration(void)
{
    if (!esl_led_state_get(blink_context.led_idx) && blink_context.state == READY_TO_ON)
    {
        esl_led_on(blink_context.led_idx);
        blink_context.delay_ms = 500;
    }

    while (blink_context.delay_ms > 0 && esl_button_state_get())
    {
        esl_delay_ms(1);
        blink_context.delay_ms--;
    }

    if (blink_context.delay_ms == 0)
    {
        blink_context.state = READY_TO_OFF;
    }
    else
    {
        return;
    }
    
    if (esl_led_state_get(blink_context.led_idx) && blink_context.state == READY_TO_OFF)
    {
        esl_led_off(blink_context.led_idx);
        blink_context.delay_ms = 500;
    }

    while (blink_context.delay_ms > 0 && esl_button_state_get())
    {
        esl_delay_ms(1);
        blink_context.delay_ms--;
    }

    if (blink_context.delay_ms == 0)
    {
        blink_context.state = READY_TO_ON;
    }
    else
    {
        return;
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