#pragma once
#include "color.h"
enum {
    BSFILL_SOLID_COLOR,
    BSFILL_COUNT
};
typedef struct BSFill {
    int kind;
    union {
        BSColor color;
    } as;
} BSFill;
