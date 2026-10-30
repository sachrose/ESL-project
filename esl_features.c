#include "esl_features.h"
#include "esl_button.h"

static blink_context_t blink_context = 
{
    .how_much_is_the_fish = {6, 6, 1, 3},
    .delay_ms = 0,
    .led_idx = LED_IDX_YELLOW_GREEN,
    .count = 6,
    .half_iteration = false
};

void led_blinking_mode_button(void)
{
    if (esl_button_state_get())
    {
        blink_iteration();
    }  
} 

void blink_iteration(void)
{
    if (blink_context.delay_ms == 0)    // после обычного завершения функции
    {
        esl_led_invert(blink_context.led_idx);
        blink_context.half_iteration = !blink_context.half_iteration;
        blink_context.delay_ms = 500;
    }
    
    while (blink_context.delay_ms > 0 && esl_button_state_get())    
    {
        esl_delay_ms(1);
        blink_context.delay_ms--;
    }

    if (blink_context.delay_ms > 0)
    {
        return;
    }  

    if (blink_context.count > 0 && !blink_context.half_iteration)
    {
        blink_context.count--;
    }
    
   
    if (blink_context.count == 0)
    {
        esl_led_off(blink_context.led_idx);
        blink_context.led_idx = (blink_context.led_idx + 1) % ESL_LEDS_NUMBER;
        blink_context.count = blink_context.how_much_is_the_fish[blink_context.led_idx];
    }
}