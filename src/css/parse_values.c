#include "parse_values.h"
#include "../todo.h"
#include <string.h>
#include <ctype.h>
#include "parser.h"
#include "log.h"

int css_parse_float(const char* css_content, const char* css_content_end, const char** end, float* result) {
    float sign = 1.f;
    if(css_content < css_content_end && *css_content == '-') {
        sign = -sign;
        css_content++;
    }
    float value = 0.f;
    while(css_content < css_content_end && isdigit(*css_content)) {
        value = value * 10.f + (*css_content - '0');
        css_content++;
    }
    float decimal = 0.1f;
    if(css_content < css_content_end && *css_content == '.') {
        css_content++;
        while(css_content < css_content_end && isdigit(*css_content)) {
            value += (*css_content - '0') * decimal;
            css_content++;
            decimal *= 0.1f;
        }
    }
    *result = value * sign;
    *end = css_content;
    return 0;
}
int css_compute_numeric(float rootFontSize, float parentFontSize, const char* css_content, const char* css_content_end, const char** end, float* result) {
    int e;
    *result = 0.0;
    float number;
    if((e=css_parse_float(css_content, css_content_end, &css_content, &number)) < 0) return e;
    const char* unit_str = css_content;
    while(css_content < css_content_end && isalnum(*css_content)) css_content++;
    size_t unit_len = css_content - unit_str;
    if(unit_len == 3 && memcmp(unit_str, "rem", 3) == 0) {
        *result = number * rootFontSize;
    } else if (unit_len == 2 && memcmp(unit_str, "em", 2) == 0) {
        *result = number * parentFontSize;
    } else if (unit_len == 2 && memcmp(unit_str, "px", 2) == 0) {
        *result = number;
    } else if (unit_len) {
        dcss_warn("Unsupported unit `%.*s`", (int)unit_len, unit_str);
    } else *result = number;
    *end = css_content;
    return 0;
}

uint8_t css_parse_decimal_byte(const char* css_content, const char* css_content_end, const char** end) {
    uint8_t value = 0;
    while(css_content < css_content_end && isdigit(*css_content)) value = (value * 10) + (*css_content++) - '0';
    *end = css_content;
    return value;
}
static uint8_t xdigit2value(int digit) {
    return digit <= '9' ? digit - '0' : (tolower(digit) - 'a' + 10);
}
#define STRLEN(x) (sizeof(x) - 1)
int css_compute_color(const char* css_content, const char* css_content_end, const char** end, CSSColor* result) {
    const char* word = css_content;
    while(css_content < css_content_end && isalnum(css_content[0])) css_content++;
    size_t word_len = css_content - word;
    CSSColor value = 0;
    if(word_len == 3 && memcmp(word, "rgb", 3) == 0) {
        css_content = css_skip(css_content, css_content_end);
        if(css_content >= css_content_end || *css_content++ != '(') return -CSSERR_INVALID_ARG_SYNTAX;
        for(size_t i = 0; i < 3; ++i) {
            css_content = css_skip(css_content, css_content_end);
            if(i > 0) {
                if(css_content >= css_content_end || *css_content++ != ',') return -CSSERR_INVALID_ARG_SYNTAX;
                css_content = css_skip(css_content, css_content_end);
            }
            value = (value << 8) | css_parse_decimal_byte(css_content, css_content_end, &css_content);
        }
        if(css_content >= css_content_end || *css_content++ != ')') return -CSSERR_INVALID_ARG_SYNTAX;
        value = (value << 8) | 0xFF;
    } else if (word_len == 0 && word + 4 <= css_content_end && *word == '#') {
        if(!isxdigit(css_content[1]) || !isxdigit(css_content[2]) || !isxdigit(css_content[3])) return -CSSERR_INVALID_ARG_SYNTAX;
        uint8_t rb = xdigit2value(css_content[1]);
        uint8_t gb = xdigit2value(css_content[2]);
        uint8_t bb = xdigit2value(css_content[3]);
        value = 
              (rb << 28) | (rb << 24)
            | (gb << 20) | (gb << 16)
            | (bb << 12) | (bb << 8) 
            | 0xFF;
        css_content += 4;
    } else {
        static struct {
            const char* name;
            CSSColor color;
            size_t len;
        } named_colors[] = {
            { "black"  , 0x000000FF, STRLEN("black"  ) },
            { "silver" , 0xC0C0C0FF, STRLEN("silver" ) },
            { "gray"   , 0x808080FF, STRLEN("gray"   ) },
            { "white"  , 0xFFFFFFFF, STRLEN("white"  ) },
            { "maroon" , 0x800000FF, STRLEN("maroon" ) },
            { "red"    , 0xFF0000FF, STRLEN("red"    ) },
            { "purple" , 0x800080FF, STRLEN("purple" ) },
            { "fuchsia", 0xFF00FFFF, STRLEN("fuchsia") },
            { "green"  , 0x008000FF, STRLEN("green"  ) },
            { "lime"   , 0x00FF00FF, STRLEN("lime"   ) },
            { "olive"  , 0x808000FF, STRLEN("olive"  ) },
            { "yellow" , 0xFFFF00FF, STRLEN("yellow" ) },
            { "navy"   , 0x000080FF, STRLEN("navy"   ) },
            { "blue"   , 0x0000FFFF, STRLEN("blue"   ) },
            { "teal"   , 0x008080FF, STRLEN("teal"   ) },
            { "aqua"   , 0x00FFFFFF, STRLEN("aqua"   ) },
        };
        for(size_t i = 0; i < sizeof(named_colors)/sizeof(*named_colors); ++i) {
            if(word_len == named_colors[i].len && memcmp(word, named_colors[i].name, word_len) == 0) {
                value = named_colors[i].color;
                goto end;
            }
        }
        dcss_warn("Unsupported color word `%.*s`", (int)word_len, word);
    } 
end:
    *result = value;
    *end = css_content;
    return 0;
}
