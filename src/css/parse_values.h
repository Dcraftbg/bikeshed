#pragma once
int css_parse_float(const char* css_content, const char* css_content_end, const char** end, float* result);
int css_compute_numeric(float rootFontSize, float parentFontSize, const char* css_content, const char* css_content_end, const char** end, float* result);
#include <stdint.h>

// RGBA 32 bit integer 
typedef uint32_t CSSColor;
int css_compute_color(const char* css_content, const char* css_content_end, const char** end, CSSColor* result);
