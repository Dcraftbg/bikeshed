#include "layouter.h"
#include "../html.h"
#include "../css/pattern_map.h"
#include "../todo.h"
#include <assert.h>
#include <string.h>
#include "../atom.h"
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include "../bsrenderer/renderer.h"
void compute_box_html_tag(BSRenderer* renderer, HTMLTag* tag, BSFont* font, float fontSize, float textFontSize, float spacing, float screen_width, size_t* cursor_x, size_t* cursor_y) {
    size_t new_x = tag->x = *cursor_x;
    size_t new_y = tag->y = *cursor_y;
    size_t max_x = tag->x;
    size_t max_y = tag->y;
    if(tag->name) {
        if(strcmp(tag->name->data, "script") == 0) return;
        for(size_t i = 0; i < tag->children.len; ++i) {
            HTMLTag* child = tag->children.items[i];
            if(child->display == CSSDISPLAY_BLOCK) {
                new_x = tag->x;
                new_y = max_y;
            }
            compute_box_html_tag(renderer, child, font, fontSize, tag->fontSize, spacing, screen_width, &new_x, &new_y);
            if(child->x + child->width > max_x) max_x = child->x + child->width;
            if(child->y + child->height > max_y) max_y = child->y + child->height;
        }
    } else {
        float x = new_x;
        float y = new_y;
        float max_width = screen_width;
        for(size_t i = 0; i < tag->str_content_len; ++i) {
            char c = tag->str_content[i];
            if(isspace(c)) {
                while(i+1 < tag->str_content_len && isspace(tag->str_content[i+1])) i++;
                c = ' ';
            } else if (!isgraph(c)) c = '?';
            BSCodepointSize size = bsrenderer_measure_codepoint(renderer, font, c, textFontSize, spacing);
            // Vector2 size = MeasureCodepointEx(font, c, textFontSize, spacing);
            if(x + size.width > max_width) {
                // TODO: unhardcode this?
                x = 0;// tag->x;
                y += textFontSize;
            }
            if(x + size.width > max_x) max_x = ceilf(x + size.width);
            // FIXME: I don't know if this is correct if it already scrolled?
            if(y + size.height > max_y) max_y = ceilf(y + size.height);
            x += size.width;
        }
        new_x = x;
        new_y = y;
    }
    tag->width = max_x - tag->x;
    tag->height = max_y - tag->y;
    static_assert(CSSDISPLAY_COUNT == 3, "Update compute_box_html_tag");
    switch(tag->display) {
    case CSSDISPLAY_BLOCK:
        *cursor_y = max_y;
        *cursor_x = tag->x;
        break;
    case CSSDISPLAY_INLINE:
        *cursor_x = new_x;
        *cursor_y = new_y;
        break;
    case CSSDISPLAY_INLINE_BLOCK:
        *cursor_x = max_x;
        *cursor_y = tag->y;
        break;
    default:
        todof("handle display: %d", tag->display);
    }
}
