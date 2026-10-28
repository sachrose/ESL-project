#ifndef LED_H
#define LED_H

#include <stdint.h>

#define LED_YELLOW_GREEN    0
#define LED_RED             1
#define LED_GREEN           2
#define LED_BLUE            3

void led_blinking_mode_device_id (void);

void led_blinking (uint32_t led_idx, uint32_t blinks_num);

#endif