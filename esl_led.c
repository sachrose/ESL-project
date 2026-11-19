#include "app_timer.h"
#include "nrf_pwm.h"
#include "nrfx_pwm.h"
#include "esl_led.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

static const uint16_t esl_device_id[ESL_LEDS_NUMBER] = ESL_DEVICE_ID;

volatile static led_color_context_t led_color_context =
{
    .state = VIEWING,
    .event = STOP_EDITING,
    .status_duty = 0,
    .hue = ESL_DEFAULT_HUE(esl_device_id[2], esl_device_id[3]),
    .sat = ESL_DEFAULT_SATURATION,
    .val = ESL_DEFAULT_VALUE,
    .status_phase_down = false,
    .h_phase_down = false,
    .s_phase_down = false,
    .v_phase_down = false
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

static char* led_state_names[LED_STATE_COUNT] = 
{
    [VIEWING] = "VIEWING",
    [EDITING_H] = "EDITING_H",
    [EDITING_S] = "EDITING_S",
    [EDITING_V] = "EDITING_V"
};

APP_TIMER_DEF(pwm_0_tick);

static void smooth_value_cycle(volatile uint16_t *value, volatile bool *phase_down, uint16_t step, uint16_t top_value)
{
    if (value == NULL || phase_down == NULL)
    {
        NRF_LOG_DEBUG("NULL pointer!");
        goto smooth_value_cycle_end;
    }
        
    if (*phase_down)
    {
        if (*value <= step)
        {
            *value = 0;
            *phase_down = false;
        }
        else
        {
            *value -= step;
        }
    }
    else
    {
        if (*value + step >= top_value)
        {
            *value = top_value;
            *phase_down = true;
        }
        else
        {
            *value += step;
        }
    }

smooth_value_cycle_end:
    return;
}

static void update_pwm(void)
{
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

    pwm_0_seq_values.channel_0 = led_color_context.status_duty;
    pwm_0_seq_values.channel_1 = (uint16_t)((r * NRFX_PWM_DEFAULT_CONFIG_TOP_VALUE) / 100);
    pwm_0_seq_values.channel_2 = (uint16_t)((g * NRFX_PWM_DEFAULT_CONFIG_TOP_VALUE) / 100);
    pwm_0_seq_values.channel_3 = (uint16_t)((b * NRFX_PWM_DEFAULT_CONFIG_TOP_VALUE) / 100);
    
    return;
}

static void state_viewing_handler(led_event_t event)
{
    led_color_context.status_duty = 0;
}

static void state_editing_h_handler(led_event_t event)
{        
    if (event == START_EDITING || event == STOP_EDITING)
    {
        led_color_context.event = event;
    }
    else if (event == PWM_TICK)
    {
        smooth_value_cycle(&led_color_context.status_duty,
                           &led_color_context.status_phase_down,
                           ESL_STATUS_STEP_SLOW,
                           NRFX_PWM_DEFAULT_CONFIG_TOP_VALUE);
        
        if (led_color_context.event == START_EDITING)
        {
            smooth_value_cycle(&led_color_context.hue,
                           &led_color_context.h_phase_down,
                           ESL_HSV_STEP,
                           MAX_HUE);
        }
    }
}

static void state_editing_s_handler(led_event_t event)
{
    if (event == START_EDITING || event == STOP_EDITING)
    {
        led_color_context.event = event;
    }
    else if (event == PWM_TICK)
    {
        smooth_value_cycle(&led_color_context.status_duty,
                           &led_color_context.status_phase_down,
                           ESL_STATUS_STEP_FAST,
                           NRFX_PWM_DEFAULT_CONFIG_TOP_VALUE);

        if (led_color_context.event == START_EDITING)
        {
            smooth_value_cycle(&led_color_context.sat,
                           &led_color_context.s_phase_down,
                           ESL_HSV_STEP,
                           MAX_SAT);
        }
        
    }
}

static void state_editing_v_handler(led_event_t event)
{
    led_color_context.status_duty = NRFX_PWM_DEFAULT_CONFIG_TOP_VALUE;

    if (event == START_EDITING || event == STOP_EDITING)
    {
        led_color_context.event = event;
    }
    else if (event == PWM_TICK && led_color_context.event == START_EDITING)
    {
        smooth_value_cycle(&led_color_context.val,
                           &led_color_context.v_phase_down,
                           ESL_HSV_STEP,
                           MAX_VAL);
    }
}

static led_state_handler_t led_state_handlers[LED_STATE_COUNT] =
{
    state_viewing_handler,
    state_editing_h_handler,
    state_editing_s_handler,
    state_editing_v_handler
};

static void esl_fsm_led(led_event_t event)
{
    if ( led_color_context.state == VIEWING && (event == START_EDITING || event == STOP_EDITING))
    {
        goto esl_fsm_led_end;
    }
    
    if (event == CHANGE_MODE)
    {
        led_state_t last_state = led_color_context.state;
        led_color_context.state = (led_color_context.state + 1) % LED_STATE_COUNT;
        NRF_LOG_INFO("New state: %s", led_state_names[led_color_context.state]);

        if (last_state == VIEWING)
        {
            app_timer_start(pwm_0_tick, ESL_PWM_TICK, NULL);            
        }
        else if (led_color_context.state == VIEWING)
        {
            app_timer_stop(pwm_0_tick);
        }
    }
    else if (event == START_EDITING)
    {
        NRF_LOG_INFO("Start editing!")
    }
    else if (event == STOP_EDITING)
    {
        NRF_LOG_INFO("Stop editing! Current color: H = %d, S = %d, V = %d", led_color_context.hue, led_color_context.sat, led_color_context.val);
    }

    led_state_handlers[led_color_context.state](event);

    if (event == PWM_TICK || event == CHANGE_MODE)
    {
        update_pwm();
    }

esl_fsm_led_end:
    return;
}

static void pwm_0_tick_handler(void * p_context)
{
    esl_fsm_led(PWM_TICK);
}

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

void change_mode(void)
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