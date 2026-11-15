#ifndef LED_H
#define LED_H

#include <stdint.h>
#include "nrf_gpio.h"
#include "nrfx_glue.h"
#include "app_timer.h"
#include "nrf_pwm.h"
#include "nrf_drv_pwm.h"

#define ESL_DEVICE_ID   {6, 6, 1, 3}
#define ESL_LEDS_NUMBER 4

#define LED1_G         NRF_GPIO_PIN_MAP(0,6)
#define LED2_R         NRF_GPIO_PIN_MAP(0,8)
#define LED2_G         NRF_GPIO_PIN_MAP(1,9)
#define LED2_B         NRF_GPIO_PIN_MAP(0,12)

#define ESL_DEFAULT_HUE(a, b)   ((uint16_t)((360 * ((a) * 10 + (b)) + 50) / 100))
#define ESL_DEFAULT_SATURATION  100
#define ESL_DEFAULT_VALUE       100

#define ESL_PWM_TICK    APP_TIMER_TICKS(20)
#define ESL_STEP_SLOW   20
#define ESL_STEP_FAST   50

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
    CHANGE_MODE = 0,
    START_EDITING = 1,
    STOP_EDITING = 2,
    PWM_TICK = 3,
    LED_EVENT_COUNT = 4
} led_event_t;

typedef struct
{
    volatile led_state_t state;
    volatile led_event_t event;
    uint16_t status_duty;
    uint16_t hue;
    uint16_t sat;
    uint16_t val;
    uint16_t r;
    uint16_t g;
    uint16_t b;
    bool phase_down;
} led_color_context_t;

typedef void (*led_state_handler_t)(led_event_t event);

void esl_pwm_led_config(void);

void esl_led_state_increment(void);

void start_editing(void);

void stop_editing(void);

#endif