#include "esl_button.h"

void esl_button_init(void)
{
    nrf_gpio_cfg_input(BUTTON, BUTTON_PULL);
}

bool esl_button_state_get(void)
{
    bool pin_set = nrf_gpio_pin_read(BUTTON) ? true : false;
    return !pin_set;
}