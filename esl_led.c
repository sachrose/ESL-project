#include "nrf_pwm.h"
#include "nrfx_pwm.h"
#include "esl_led.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

static void esl_fsm_led(led_event_t event);

static led_color_context_t led_color_context =
{
    .save_color_callback = NULL,
    .state = VIEWING,
    .event = STOP_EDITING,
    .phase_down.status = false,
    .phase_down.hue = false,
    .phase_down.sat = false,
    .phase_down.val = false,
    .status_duty = 0 
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

static char* led_event_names[LED_EVENT_COUNT] =
{
    [VIEW_COLOR] = "Current color: ",
    [CHANGE_MODE] = "Change mode!",
    [START_EDITING] = "Start editing!",
    [STOP_EDITING] = "Stop editing!",
    [PWM_TICK] = "PWM tick"
};

APP_TIMER_DEF(pwm_0_tick);

#if ESTC_USB_CLI_ENABLED
static int32_t max_value_3(int32_t a, int32_t b, int32_t c)
{
    int32_t res = (a > b) ? a : b;
    return (res > c) ? res : c;
}

static int32_t min_value_3(int32_t a, int32_t b, int32_t c)
{
    int32_t res = (a < b) ? a : b;
    return (res < c) ? res : c;
}

static void rgb_to_hsv(uint8_t red, uint8_t green, uint8_t blue)
{
    int32_t r = red;
    int32_t g = green;
    int32_t b = blue;
    int32_t hue = MIN_COLOR_VALUE;
    int32_t max_val = max_value_3(r, g, b);
    int32_t min_val = min_value_3(r, g, b);
    int32_t delta = max_val - min_val;

    led_color_context.new_color.val = (uint8_t)((max_val * 100) / 255);

    if (max_val == MIN_COLOR_VALUE || delta == MIN_COLOR_VALUE)
    {
        led_color_context.new_color.sat = MIN_COLOR_VALUE;
        led_color_context.new_color.hue = MIN_COLOR_VALUE;
        goto rgb_to_hsv_end;
    }

    led_color_context.new_color.sat = (uint8_t)((delta * 100) / max_val);

    if (max_val == r)
    {
        hue = (60 * (g - b)) / delta;
        if (hue < MIN_COLOR_VALUE)
        {
            hue += 360;
        }
    }
    else if (max_val == g)
    {
        hue = 120 + (60 * (b - r)) / delta;
    }
    else
    {
        hue = 240 + (60 * (r - g)) / delta;
    }

    led_color_context.new_color.hue = (uint16_t)hue;

rgb_to_hsv_end:
    return;
}
#endif

static void hsv_to_rgb(uint32_t* r, uint32_t* g, uint32_t* b)
{    
    if (led_color_context.new_color.sat == MIN_COLOR_VALUE) 
    {
        *r = *g = *b = led_color_context.new_color.val;
        goto hsv_to_rgb_end;
    }

    uint16_t h = led_color_context.new_color.hue % 360;
    uint8_t region = h / 60;
    uint16_t remainder = h % 60;
    
    uint32_t p = (led_color_context.new_color.val * (100 - led_color_context.new_color.sat)) / 100;
    uint32_t q = (led_color_context.new_color.val * (6000 - led_color_context.new_color.sat * remainder)) / 6000;
    uint32_t t = (led_color_context.new_color.val * (6000 - led_color_context.new_color.sat * (60 - remainder))) / 6000;

    switch (region)
    {
        case 0:
            *r = led_color_context.new_color.val; *g = t; *b = p;
            break;
        case 1:
            *r = q; *g = led_color_context.new_color.val; *b = p;
            break;
        case 2:
            *r = p; *g = led_color_context.new_color.val; *b = t;
            break;
        case 3:
            *r = p; *g = q; *b = led_color_context.new_color.val;
            break;
        case 4:
            *r = t; *g = p; *b = led_color_context.new_color.val;
            break;
        default:
            *r = led_color_context.new_color.val; *g = p; *b = q;
            break;
    }

hsv_to_rgb_end:
    return;
}

static void set_color_to_channels(uint32_t r, uint32_t g, uint32_t b)
{
    pwm_0_seq_values.channel_0 = led_color_context.status_duty;
    pwm_0_seq_values.channel_1 = (uint16_t)((r * NRFX_PWM_DEFAULT_CONFIG_TOP_VALUE) / 100);
    pwm_0_seq_values.channel_2 = (uint16_t)((g * NRFX_PWM_DEFAULT_CONFIG_TOP_VALUE) / 100);
    pwm_0_seq_values.channel_3 = (uint16_t)((b * NRFX_PWM_DEFAULT_CONFIG_TOP_VALUE) / 100); 
}

static void update_pwm(void)
{
    uint32_t r, g, b;
    
    hsv_to_rgb(&r, &g, &b);   
    set_color_to_channels(r, g, b);
}

static bool is_color_diff(hsv_color_t current, hsv_color_t new)
{
    return (current.hue != new.hue) || (current.sat != new.sat) || (current.val != new.val);
}

static void smooth_value_cycle(volatile void* value_ptr,
                               bool is_uint16,
                               volatile bool* phase_down,
                               uint16_t step,
                               uint16_t top_value)
{   
    if (value_ptr == NULL || phase_down == NULL)
    {
        NRF_LOG_DEBUG("NULL pointer!");
        goto smooth_value_cycle_end;
    }
    
    uint32_t value = is_uint16 ? *(volatile uint16_t*)value_ptr : *(volatile uint8_t*)value_ptr;
    
    if (*phase_down)
    {
        *phase_down = (value > step);
        value = *phase_down ? (value - step) : MIN_COLOR_VALUE;
    }
    else
    {
        *phase_down = (value + step >= top_value);
        value = *phase_down ? top_value : (value + step);
    }

    if (is_uint16)
    {
        *(volatile uint16_t*)value_ptr = (uint16_t)value;
    }
    else
    {
        *(volatile uint8_t*)value_ptr = (uint8_t)value;
    }

smooth_value_cycle_end:
    return;
}

static void log_current_color(void)
{
    NRF_LOG_INFO("Current color: H = %d, S = %d, V = %d",
                 led_color_context.new_color.hue,
                 led_color_context.new_color.sat,
                 led_color_context.new_color.val);
}

static void state_viewing_handler(led_event_t event)
{
    if (is_color_diff(led_color_context.current_color, led_color_context.new_color))
    {
        log_current_color();

        led_color_context.current_color = led_color_context.new_color;

        if (led_color_context.save_color_callback != NULL)
        {
            led_color_context.save_color_callback(led_color_context.new_color);
        }

        NRF_LOG_INFO("New color saved!");
    }
}

static void state_editing_h_handler(led_event_t event)
{        
    if (event == PWM_TICK)
    {
        SMOOTH_VALUE_CYCLE(&led_color_context.status_duty,
                           &led_color_context.phase_down.status,
                           ESL_STATUS_STEP_SLOW,
                           NRFX_PWM_DEFAULT_CONFIG_TOP_VALUE);
        
        if (led_color_context.event == START_EDITING)
        {
            SMOOTH_VALUE_CYCLE(&led_color_context.new_color.hue,
                               &led_color_context.phase_down.hue,
                               ESL_HSV_STEP,
                               MAX_HUE);
        }
    }
}

static void state_editing_s_handler(led_event_t event)
{
    if (event == PWM_TICK)
    {
        SMOOTH_VALUE_CYCLE(&led_color_context.status_duty,
                           &led_color_context.phase_down.status,
                           ESL_STATUS_STEP_FAST,
                           NRFX_PWM_DEFAULT_CONFIG_TOP_VALUE);

        if (led_color_context.event == START_EDITING)
        {
            SMOOTH_VALUE_CYCLE(&led_color_context.new_color.sat,
                               &led_color_context.phase_down.sat,
                               ESL_HSV_STEP,
                               MAX_SAT);
        }
    }
}

static void state_editing_v_handler(led_event_t event)
{
    if (event == PWM_TICK && led_color_context.event == START_EDITING)
    {
        SMOOTH_VALUE_CYCLE(&led_color_context.new_color.val,
                           &led_color_context.phase_down.val,
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
    static uint32_t log_counter = 0;
    led_state_t last_state;
    
    if ( led_color_context.state == VIEWING && (event == START_EDITING || event == STOP_EDITING))
    {
        goto esl_fsm_led_end;
    }

    if (event == VIEW_COLOR)
    {
        NRF_LOG_INFO("Current color: H = %d, S = %d, V = %d",
                     led_color_context.new_color.hue,
                     led_color_context.new_color.sat,
                     led_color_context.new_color.val);
    }
    
    if (event == CHANGE_MODE)
    {
        last_state = led_color_context.state;
        led_color_context.state = (led_color_context.state + 1) % LED_STATE_COUNT;
        NRF_LOG_INFO("%s Now: %s", led_event_names[event], led_state_names[led_color_context.state]);

        if (last_state == VIEWING)
        {
            app_timer_start(pwm_0_tick, ESL_PWM_TICK, NULL);            
        }
        else if (led_color_context.state == VIEWING)
        {
            led_color_context.status_duty = MIN_COLOR_VALUE;
            app_timer_stop(pwm_0_tick);
        }
        else if (led_color_context.state == EDITING_V)
        {
            led_color_context.status_duty = NRFX_PWM_DEFAULT_CONFIG_TOP_VALUE;
        }
    }
    else if (event == START_EDITING || event == STOP_EDITING)
    {
        led_color_context.event = event;
    }
    else if (event == PWM_TICK && led_color_context.event == START_EDITING)
    {
        log_counter++;
        if (log_counter == LOG_TICK)
        {
            log_current_color();
            log_counter = 0;
        }
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

void esl_pwm_led_config(hsv_color_t start_color, led_save_color_callback_t save_color)
{          
    pwm_0_cfg.load_mode = NRF_PWM_LOAD_INDIVIDUAL;
    pwm_0_cfg.output_pins[0] = LED1_G | NRFX_PWM_PIN_INVERTED;
    pwm_0_cfg.output_pins[1] = LED2_R | NRFX_PWM_PIN_INVERTED;
    pwm_0_cfg.output_pins[2] = LED2_G | NRFX_PWM_PIN_INVERTED;
    pwm_0_cfg.output_pins[3] = LED2_B | NRFX_PWM_PIN_INVERTED;

    app_timer_create(&pwm_0_tick, APP_TIMER_MODE_REPEATED, pwm_0_tick_handler);

    nrfx_pwm_init(&pwm_0, &pwm_0_cfg, NULL);

    led_color_context.save_color_callback = save_color;
    led_color_context.current_color = start_color;
    led_color_context.new_color = start_color;

    update_pwm();
    
    nrfx_pwm_simple_playback(&pwm_0, &seq_0, 1, NRFX_PWM_FLAG_LOOP);
}

void view_current_color(void)
{
    esl_fsm_led(VIEW_COLOR);
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

#if ESTC_USB_CLI_ENABLED
void rgb_from_cli(uint8_t r, uint8_t g, uint8_t b)
{
    rgb_to_hsv(r, g, b);
    update_pwm();
}

void hsv_from_cli(uint16_t h, uint8_t s, uint8_t v)
{
    led_color_context.new_color.hue = h;
    led_color_context.new_color.sat = s;
    led_color_context.new_color.val = v;
    update_pwm();
}
#endif