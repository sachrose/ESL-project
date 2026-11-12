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

#define ESL_PWM_LED_SEQ_STEPS           20
#define ESL_PWM_LED_STEPS_ARR_SIZE      (ESL_PWM_LED_SEQ_STEPS & 1 ? ESL_PWM_LED_SEQ_STEPS : (ESL_PWM_LED_SEQ_STEPS + 1))
#define ESL_PWM_LED_ONE_STEP_DUTY       (NRFX_PWM_DEFAULT_CONFIG_TOP_VALUE / ESL_PWM_LED_SEQ_STEPS)


#define ESL_DEFAULT_HUE(a, b)   ((uint16_t)((360 * ((a) * 10 + (b)) + 50) / 100))
#define ESL_DEFAULT_SATURATION  100
#define ESL_DEFAULT_VALUE       100

typedef struct
{
    //volatile led_state_t state;
    uint16_t hue;
    uint8_t sat;
    uint8_t val;
    uint8_t r;
    uint8_t g;
    uint8_t b;
} hsv_context_t;

void esl_pwm_led_init(void);

#endif