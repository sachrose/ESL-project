#ifndef ESL_TYPES_H
#define ESL_TYPES_H

#include <stdint.h>

#define MAX_HUE 360
#define MAX_SAT 100
#define MAX_VAL 100

#define LOG_TICK 10

typedef struct
{
    uint16_t hue;
    uint8_t  sat;
    uint8_t  val;
} hsv_color_t;

typedef hsv_color_t hsv_word_t;

#endif