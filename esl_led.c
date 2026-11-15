#include "nrf_assert.h"
#include "app_timer.h"
#include "nrf.h"
#include "nrf_pwm.h"
#include "nrfx_pwm.h"
#include "esl_led.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

void esl_fsm_led(led_event_t event);

void pwm_0_tick_handler(void * p_context);

void update_pwm(void);

void esl_fsm_viewing_state(led_event_t event);

void esl_fsm_editing_h_state(led_event_t event);

void esl_fsm_editing_s_state(led_event_t event);

void esl_fsm_editing_v_state(led_event_t event);

static const uint16_t esl_device_id[ESL_LEDS_NUMBER] = ESL_DEVICE_ID;

static led_color_context_t led_color_context =
{
    .state = VIEWING,
    .event = STOP_EDITING,
    .status_duty = 0,
    .hue = ESL_DEFAULT_HUE(esl_device_id[2], esl_device_id[3]),
    .sat = ESL_DEFAULT_SATURATION,
    .val = ESL_DEFAULT_VALUE,
    .phase_down = false
};

static nrfx_pwm_t pwm_0 = NRFX_PWM_INSTANCE(0);
static nrfx_pwm_config_t pwm_0_cfg = NRFX_PWM_DEFAULT_CONFIG;
static nrf_pwm_values_individual_t pwm_0_seq_values;
static nrf_pwm_sequence_t seq_0 =
{
    .values.p_individual = &pwm_0_seq_values,
    .length = NRF_PWM_VALUES_LENGTH(pwm_0_seq_values),
    .repeats = 0,
    .end_delay = 0
};

static led_state_handler_t led_state_handlers[LED_STATE_COUNT] =
{
    esl_fsm_viewing_state,
    esl_fsm_editing_h_state,
    esl_fsm_editing_s_state,
    esl_fsm_editing_v_state
};

APP_TIMER_DEF(pwm_0_tick);

void esl_pwm_led_config(void)
{          
    pwm_0_cfg.load_mode = NRF_PWM_LOAD_INDIVIDUAL;
    pwm_0_cfg.output_pins[0] = LED1_G | NRFX_PWM_PIN_INVERTED;
    pwm_0_cfg.output_pins[1] = LED2_R | NRFX_PWM_PIN_INVERTED;
    pwm_0_cfg.output_pins[2] = LED2_G | NRFX_PWM_PIN_INVERTED;
    pwm_0_cfg.output_pins[3] = LED2_B | NRFX_PWM_PIN_INVERTED;

    app_timer_create(&pwm_0_tick, APP_TIMER_MODE_REPEATED, pwm_0_tick_handler);

    nrfx_pwm_init(&pwm_0, &pwm_0_cfg, NULL);
    update_pwm();
    
    nrfx_pwm_simple_playback(&pwm_0, &seq_0, 1, NRFX_PWM_FLAG_LOOP);
}

void pwm_0_tick_handler(void * p_context)
{
    esl_fsm_led(PWM_TICK);
}

void esl_fsm_led(led_event_t event)
{
    if (event == CHANGE_MODE)
    {
        led_state_t last_state = led_color_context.state;
        led_color_context.state = (led_color_context.state + 1) % LED_STATE_COUNT;
        NRF_LOG_INFO("New state: %d", led_color_context.state);

        if (last_state == VIEWING)
        {
            app_timer_start(pwm_0_tick, ESL_PWM_TICK, NULL);
        }
        else if (led_color_context.state == VIEWING)
        {
            app_timer_stop(pwm_0_tick);
        }
    } 

    led_state_handlers[led_color_context.state](event);

    if (event == PWM_TICK)
    {
        update_pwm();
    }
}

void esl_led_state_increment(void)
{
   esl_fsm_led(CHANGE_MODE);
}

void start_editing(void)
{
    esl_fsm_led(START_EDITING);
}

void stop_editing(void)
{
    esl_fsm_led(STOP_EDITING);
}

void count_status_duty(uint16_t step)
{
    if (led_color_context.phase_down)
    {
        led_color_context.status_duty -= step;
        if (led_color_context.status_duty <= step)
        {
            led_color_context.status_duty = 0;
            led_color_context.phase_down = false;
        }        
    }
    else
    {
        led_color_context.status_duty += step;
        if (led_color_context.status_duty >= NRFX_PWM_DEFAULT_CONFIG_TOP_VALUE)
        {
            led_color_context.status_duty = NRFX_PWM_DEFAULT_CONFIG_TOP_VALUE;
            led_color_context.phase_down = true;
        }
    }    
}

void update_pwm(void)
{
    //Status LED
    if (led_color_context.state == EDITING_H)
    {
        count_status_duty(ESL_STEP_SLOW);
    }

    if (led_color_context.state == EDITING_S)
    {
        count_status_duty(ESL_STEP_FAST);
    }
    
    //RGB LED
    uint32_t r, g, b;
    
    if (led_color_context.sat == 0) 
    {
        r = g = b = led_color_context.val;
        goto hsv_to_rgb_end;
    }

    uint16_t h = led_color_context.hue % 360;
    uint8_t region = h / 60;
    uint16_t remainder = h % 60;
    
    uint32_t p = (led_color_context.val * (100 - led_color_context.sat)) / 100;
    uint32_t q = (led_color_context.val * (6000 - led_color_context.sat * remainder)) / 6000;
    uint32_t t = (led_color_context.val * (6000 - led_color_context.sat * (60 - remainder))) / 6000;

    switch (region)
    {
        case 0:
            r = led_color_context.val; g = t; b = p;
            break;
        case 1:
            r = q; g = led_color_context.val; b = p;
            break;
        case 2:
            r = p; g = led_color_context.val; b = t;
            break;
        case 3:
            r = p; g = q; b = led_color_context.val;
            break;
        case 4:
            r = t; g = p; b = led_color_context.val;
            break;
        default:
            r = led_color_context.val; g = p; b = q;
            break;
    }
    
hsv_to_rgb_end:
    led_color_context.r = (uint16_t)((r * NRFX_PWM_DEFAULT_CONFIG_TOP_VALUE) / 100);
    led_color_context.g = (uint16_t)((g * NRFX_PWM_DEFAULT_CONFIG_TOP_VALUE) / 100);
    led_color_context.b = (uint16_t)((b * NRFX_PWM_DEFAULT_CONFIG_TOP_VALUE) / 100);

    pwm_0_seq_values.channel_0 = led_color_context.status_duty;
    pwm_0_seq_values.channel_1 = led_color_context.r;
    pwm_0_seq_values.channel_2 = led_color_context.g;
    pwm_0_seq_values.channel_3 = led_color_context.b;
    return;
}

void esl_fsm_viewing_state(led_event_t event)
{
    app_timer_stop(pwm_0_tick);
    led_color_context.status_duty = 0;
    pwm_0_seq_values.channel_0 = led_color_context.status_duty;
}

void esl_fsm_editing_h_state(led_event_t event)
{    
    if (event == START_EDITING || event == STOP_EDITING)
    {
        led_color_context.event = event;
        NRF_LOG_INFO("Edit H event: %d", event);
    }
    else if (event == PWM_TICK && led_color_context.event == START_EDITING)
    {
        led_color_context.hue = (led_color_context.hue + 1) % 360;
    }
}

void esl_fsm_editing_s_state(led_event_t event)
{
    if (event == START_EDITING || event == STOP_EDITING)
    {
        led_color_context.event = event;
        NRF_LOG_INFO("Edit H event: %d", event);
    }
    else if (event == PWM_TICK && led_color_context.event == START_EDITING)
    {
        led_color_context.sat = (led_color_context.sat + 1) % 101;
    }
}

void esl_fsm_editing_v_state(led_event_t event)
{
    led_color_context.status_duty = NRFX_PWM_DEFAULT_CONFIG_TOP_VALUE;

    if (event == START_EDITING || event == STOP_EDITING)
    {
        led_color_context.event = event;
        NRF_LOG_INFO("Edit H event: %d", event);
    }
    else if (event == PWM_TICK && led_color_context.event == START_EDITING)
    {
        led_color_context.val = (led_color_context.val + 1) % 101;
    }
}