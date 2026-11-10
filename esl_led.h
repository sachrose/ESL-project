#ifndef LED_H
#define LED_H

#include <stdint.h>
#include "nrf_gpio.h"
#include "nrfx_glue.h"
#include "nrfx_systick.h"
#include "nrf_pwm.h"
#include "nrf_drv_pwm.h"

#define LEDS_ACTIVE_STATE       0
#define ESL_LEDS_NUMBER         4

#define LED1_G         NRF_GPIO_PIN_MAP(0,6)
#define LED2_R         NRF_GPIO_PIN_MAP(0,8)
#define LED2_G         NRF_GPIO_PIN_MAP(1,9)
#define LED2_B         NRF_GPIO_PIN_MAP(0,12)

#define ESL_DEVICE_ID {6, 6, 1, 3}
#define ESL_LEDS_LIST { LED1_G, LED2_R, LED2_G, LED2_B }

#define ESL_PWM_LED_SEQ_STEPS           50
#define ESL_PWM_LED_STEPS_ARR_SIZE      (ESL_PWM_LED_SEQ_STEPS & 1 ? ESL_PWM_LED_SEQ_STEPS : (ESL_PWM_LED_SEQ_STEPS + 1))
#define ESL_PWM_LED_ONE_STEP_DUTY       (NRFX_PWM_DEFAULT_CONFIG_TOP_VALUE / ESL_PWM_LED_SEQ_STEPS)
#define ESL_PWM_LED_DUTY_DURATION_US    20000

typedef struct
{
    uint8_t blinks[ESL_LEDS_NUMBER];
    uint16_t delay_ms;
    uint8_t led_idx;
    uint8_t count;
    bool ready_to_count;    
} blink_context_t;

typedef struct
{
    uint8_t blinks[ESL_LEDS_NUMBER];
    uint8_t led_pins[ESL_LEDS_NUMBER];
    nrfx_systick_state_t systick_state;
    uint8_t led_idx;
    uint8_t count;
    uint8_t current_step;
} smooth_blink_context_t;

typedef enum
{
    BLINK_MODE_SIMPLE,
    BLINK_MODE_SMOOTH
} led_mode_t;

extern blink_context_t blink_context;
extern smooth_blink_context_t smooth_blink_context;

#define ESL_PWM_LED_CONFIG                                                  \
{                                                                           \
    .output_pins  = { led_pin | NRFX_PWM_PIN_INVERTED,                      \
                      NRFX_PWM_PIN_NOT_USED,                                \
                      NRFX_PWM_PIN_NOT_USED,                                \
                      NRFX_PWM_PIN_NOT_USED },                              \
    .irq_priority = NRFX_PWM_DEFAULT_CONFIG_IRQ_PRIORITY,                   \
    .base_clock   = (nrf_pwm_clk_t)NRFX_PWM_DEFAULT_CONFIG_BASE_CLOCK,      \
    .count_mode   = (nrf_pwm_mode_t)NRFX_PWM_DEFAULT_CONFIG_COUNT_MODE,     \
    .top_value    = NRFX_PWM_DEFAULT_CONFIG_TOP_VALUE,                      \
    .load_mode    = NRFX_PWM_DEFAULT_CONFIG_LOAD_MODE,                      \
    .step_mode    = (nrf_pwm_dec_step_t)NRFX_PWM_DEFAULT_CONFIG_STEP_MODE,  \
}

void esl_gpio_leds_init(void);

bool esl_gpio_led_state_get(uint8_t led_idx);

void esl_gpio_led_invert(uint8_t led_idx);

void esl_gpio_change_one_led_state(uint8_t led_idx, bool leds_active_state);

void esl_gpio_change_all_leds_state(bool leds_active_state);

void esl_gpio_led_blinking_mode_device_id(void);

void esl_pwm_led_init(uint8_t led_pin);

void esl_pwm_led_seq_values_init(void);

void led_blink_iteration(void);

void led_smooth_blink_iteration(void);

__STATIC_INLINE void esl_delay_ms(uint32_t ms_time)
{
    if (ms_time == 0)
    {
        return;
    }

    do {
        NRFX_DELAY_US(1000);
    } while (--ms_time);
}

#endif