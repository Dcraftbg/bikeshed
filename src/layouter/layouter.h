#pragma once
#include <stddef.h>
typedef struct BSFont BSFont;
typedef struct BSRenderer BSRenderer;
typedef struct HTMLTag HTMLTag;
void compute_box_html_tag(BSRenderer* renderer, HTMLTag* tag, BSFont* font, float fontSize, float textFontSize, float spacing, float screen_width, size_t* cursor_x, size_t* cursor_y);
