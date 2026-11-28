#ifndef ESL_LED_H
#define ESL_LED_H

#include <stdint.h>
#include "esl_types.h"
#include "nrf_gpio.h"
#include "app_timer.h"

#define ESL_LEDS_NUMBER     4

#define LED1_G  NRF_GPIO_PIN_MAP(0,6)
#define LED2_R  NRF_GPIO_PIN_MAP(0,8)
#define LED2_G  NRF_GPIO_PIN_MAP(1,9)
#define LED2_B  NRF_GPIO_PIN_MAP(0,12)

#define ESL_DEFAULT_HUE(a, b)   ((uint16_t)((MAX_HUE * ((a) * 10 + (b)) + 50) / 100))
#define ESL_DEFAULT_SATURATION  MAX_SAT
#define ESL_DEFAULT_VALUE       MAX_VAL

#define ESL_PWM_TICK            APP_TIMER_TICKS(20)
#define ESL_STATUS_STEP_SLOW    20
#define ESL_STATUS_STEP_FAST    50
#define ESL_HSV_STEP            1

#define SMOOTH_VALUE_CYCLE(value_ptr, phase_ptr, step, top_value) \
        smooth_value_cycle((value_ptr), (sizeof(*(value_ptr)) == sizeof(uint16_t)), (phase_ptr), (step), (top_value))

typedef enum
{
    VIEWING = 0,
    EDITING_H = 1,
    EDITING_S = 2,
    EDITING_V = 3,
    LED_STATE_COUNT = 4
} led_state_t;

typedef enum
{
    VIEW_COLOR = 0,
    CHANGE_MODE = 1,
    START_EDITING = 2,
    STOP_EDITING = 3,
    PWM_TICK = 4,
    LED_EVENT_COUNT = 5
} led_event_t;

typedef struct
{
    bool status;
    bool hue;
    bool sat;
    bool val;
} led_phase_t;

typedef void (*led_state_handler_t)(led_event_t event);
typedef void (*led_save_color_callback_t)(hsv_color_t new_color);

typedef struct
{
    led_save_color_callback_t save_color_callback;
    volatile led_state_t state;
    volatile led_event_t event;
    volatile hsv_color_t current_color;
    volatile hsv_color_t new_color;
    volatile led_phase_t phase_down;
    volatile uint16_t status_duty;
} led_color_context_t;

void esl_pwm_led_config(hsv_color_t start_color, led_save_color_callback_t save_color);

void view_current_color(void);

void change_mode(void);

void start_editing(void);

void stop_editing(void);

#if ESTC_USB_CLI_ENABLED
void rgb_from_cli(uint8_t r, uint8_t g, uint8_t b);

void hsv_from_cli(uint16_t h, uint8_t s, uint8_t v);
#endif

#endif