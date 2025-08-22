#pragma once
#include <stddef.h>
extern size_t render_box_color_n;
typedef struct HTMLTag HTMLTag;
typedef struct BSRenderer BSRenderer;

void render_box_html_tag(BSRenderer* renderer, HTMLTag* tag, float scroll_y);
