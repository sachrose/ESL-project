/**
 * Copyright (c) 2014 - 2021, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <stdbool.h>
#include <stdint.h>
#include "esl_led.h"
#include "esl_button.h"
#include "esl_features.h"
#include "nrf_log_backend_usb.h"

int main(void)
{
    /* Configure board. */
    esl_leds_init();
    esl_button_init();

    /* Toggle LEDs. */
    while (true)
    {
        led_blinking_mode_button();     
    }
 }


 /*
 /home/user/devel/esl-nsdk/modules/nrfx/drivers/src/nrfx_gpiote.c

 err_code = nrfx_gpiote_init();
 APP_ERROR_CHECK(err_code);


 nrfx_err_t nrfx_gpiote_init(void)

 nrfx_err_t nrfx_gpiote_in_init(nrfx_gpiote_pin_t               pin,
                               nrfx_gpiote_in_config_t const * p_config,
                               nrfx_gpiote_evt_handler_t       evt_handler)

void nrfx_gpiote_in_event_enable(nrfx_gpiote_pin_t pin, bool int_enable)

typedef struct
{
    nrf_gpiote_polarity_t sense;
    nrf_gpio_pin_pull_t   pull;                
    bool                  is_watcher      : 1;
    bool                  hi_accuracy     : 1; 
    bool                  skip_gpio_setup : 1; 
} nrfx_gpiote_in_config_t;

#define NRFX_GPIOTE_CONFIG_IN_SENSE_TOGGLE(hi_accu) \
{                                                   \
    .sense = NRF_GPIOTE_POLARITY_TOGGLE,            \
    .pull = NRF_GPIO_PIN_NOPULL,                    \
    .is_watcher = false,                            \
    .hi_accuracy = hi_accu,                         \
    .skip_gpio_setup = false,                       \
}
 
 
 */

/*
#include "nrf_log_ctrl.h"
#include "nrf_log_backend_usb.h"
#include "nrf_log.h"

@def NRF_LOG_INFO
 *  @brief Macro for logging error messages. It takes a printf-like, formatted
 *  string with up to seven arguments.
 *
 *  @details This macro is compiled only if @ref NRF_LOG_LEVEL includes info logs.
 */

 /*
 /home/user/devel/esl-nsdk/components/libraries/timer/app_timer.h

 APP_TIMER_TICKS(MS) 
 */

/** @def NRF_LOG_DEBUG
 *  @brief Macro for logging error messages. It takes a printf-like, formatted
 *  string with up to seven arguments.
 *
 *  @details This macro is compiled only if @ref NRF_LOG_LEVEL includes debug logs.


#define NRF_LOG_ERROR(...)                     NRF_LOG_INTERNAL_ERROR(__VA_ARGS__)
#define NRF_LOG_WARNING(...)                   NRF_LOG_INTERNAL_WARNING( __VA_ARGS__)
#define NRF_LOG_INFO(...)                      NRF_LOG_INTERNAL_INFO( __VA_ARGS__)
#define NRF_LOG_DEBUG(...)                     NRF_LOG_INTERNAL_DEBUG( __VA_ARGS__)


#include "nrf_log_default_backends.h"
*/

 /*
volatile bool blinking_enabled = false;

volatile static irq_context_t irq_context = 
{
    .button_pressed = false;
    .time = 0;
    .clicks = 0;
};

static blink_context_t blink_context = 
{
    .blinks = ESL_DEVICE_ID,
    .delay_ms = 0,
    .led_idx = IDX_LED1_G,
    .count = 6,
    .button_state_ = true
};

void esl_hardware_init(void)
{
    esl_leds_init();
    esl_button_init();

    nrfx_gpiote_in_config_t context = NRFX_GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);
    context.pull = NRF_GPIO_PIN_PULLUP;

    nrfx_err_t nrfx_gpiote_init();
    nrfx_err_t nrfx_gpiote_in_init(BUTTON, &context, button_IRQHandler);
    void nrfx_gpiote_in_event_enable(nrfx_gpiote_pin_t pin, true)

    app_timer_init();
    APP_TIMER_DEF(debounce);
    app_timer_create(&debounce, APP_TIMER_MODE_SINGLE_SHOT, debounce_timer_IRQHandler);

    APP_TIMER_DEF(double_click);
    app_timer_create(&double_click, APP_TIMER_MODE_SINGLE_SHOT, double_click_IRQHandler);
}

void button_IRQHandler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    app_timer_stop(&debounce_timer);
    app_timer_start(&debounce, APP_TIMER_TICKS(DEBOUNCE_TIMEOUT), NULL);    
}

void debounce_timer_IRQHandler(void *irq_context)
{
    bool current_button_state = esl_button_state_get;
    if (current_button_state == .button_pressed)
    {
        return;
    }
        
    app_timer_stop(&double_click);
    app_timer_start(&double_click, APP_TIMER_TICKS(DOUBLE_CLICK_TIMEOUT), NULL);

    if (esl_button_state_get)
    {
       .button_pressed = true;
       .time = app_timer_cnt_get;
       .clicks = 1;
    }
    else
    {
        .clicks = 0
        return;
    }
}

void double_click_IRQHandler(void *irq_context)
{
 .clicks = 0;
}

 */