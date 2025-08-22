#include "apply.h"
#include "../html.h"
#include "../todo.h"
#include "../atom.h"
#include <string.h>
#include "parse_values.h"
#include "log.h"

void apply_css_styles(HTMLTag* tag, float rootFontSize) {
    tag->fontSize = rootFontSize;
    for(size_t i = 0; i < tag->css_attribs.len; ++i) { 
        CSSAttribute* att = &tag->css_attribs.items[i];
        // TODO: atomise this sheizung
        if(strcmp(att->name->data, "display") == 0) {
            if(att->args.len > 1) dcss_warn("ignoring extra args to display");
            else if(att->args.len < 1) {
                dcss_err("too few args to display!");
                continue;
            }
            CSSArg* arg = &att->args.items[i];
            if(arg->value_len == 5 && memcmp(arg->value, "block", 5) == 0) {
                tag->display = CSSDISPLAY_BLOCK;
            } else if(arg->value_len == 6 && memcmp(arg->value, "inline", 6) == 0) {
                tag->display = CSSDISPLAY_INLINE;
            } else if(arg->value_len == 12 && memcmp(arg->value, "inline-block", 12) == 0) {
                tag->display = CSSDISPLAY_INLINE_BLOCK;
            } else {
                dcss_err("display: %.*s", (int)arg->value_len, arg->value);
            }
        } else if (strcmp(att->name->data, "font") == 0) {
            switch(att->args.len) {
            case 3: {
                CSSArg font_size = att->args.items[0];
                CSSArg font = att->args.items[1];
                CSSArg family = att->args.items[2];
                dcss_todo("font: %.*s, family: %.*s", (int)font.value_len, font.value, (int)family.value_len, family.value);
                const char* css_start = font_size.value;
                const char* css_end = font_size.value + font_size.value_len;
                float parent_font_size = tag->parent->fontSize;
                int e = css_compute_numeric(rootFontSize, parent_font_size, css_start, css_end, &css_start, &tag->fontSize);
                if(e < 0) {
                    dcss_err("parsing font_size in font: %s", csserr_str(e));
                    continue;
                }
                if(css_start < css_end && css_start[0] == '/') {
                    dcss_todo("parse font-size/line height");
                    continue;
                }
            } break;
            default:
                dcss_warn("Ignoring %zu number of arguments to font: property", att->args.len);
            }
        } else if (strcmp(att->name->data, "background-color") == 0) {
            if(att->args.len > 1) dcss_warn("ignoring extra args to background-color");
            else if(att->args.len < 1) {
                dcss_err("too few args to background-color!");
                continue;
            }
            CSSArg color_arg = att->args.items[0];
            const char* css_start = color_arg.value;
            const char* css_end = color_arg.value + color_arg.value_len;
            int e = css_compute_color(css_start, css_end, &css_start, &tag->background_color);
            if(e < 0) {
                dcss_err("parsing color of background-color: %s", csserr_str(e));
                continue;
            }
            if(css_start != css_end) {
                dcss_warn("Ignoring extra characters in background-color");
            }
            // dcss_warn("background-color: `%.*s` => %08X", (int)color_arg.value_len, color_arg.value, tag->background_color);
        } else if (strcmp(att->name->data, "color") == 0) {
            if(att->args.len > 1) dcss_warn("ignoring extra args to color");
            else if(att->args.len < 1) {
                dcss_err("too few args to color!");
                continue;
            }
            CSSArg color_arg = att->args.items[0];
            const char* css_start = color_arg.value;
            const char* css_end = color_arg.value + color_arg.value_len;
            int e = css_compute_color(css_start, css_end, &css_start, &tag->color);
            if(e < 0) {
                dcss_err("parsing color of color: %s", csserr_str(e));
                continue;
            }
            if(css_start != css_end) {
                dcss_warn("Ignoring extra characters in color");
            }
            // dcss_warn("color: `%.*s` => %08X for %s", (int)color_arg.value_len, color_arg.value, tag->color, tag->name->data);
        } else if(strcmp(att->name->data, "font-size") == 0) {
            if(att->args.len > 1) dcss_warn("ignoring extra args to font-size");
            else if(att->args.len < 1) {
                dcss_err("too few args in font-size!");
                continue;
            }
            CSSArg* arg = &att->args.items[0];
            const char* _end;
            float parent_font_size = tag->parent->fontSize;
            int e = css_compute_numeric(rootFontSize, parent_font_size, arg->value, arg->value+arg->value_len, &_end, &tag->fontSize);
            if(e < 0) {
                dcss_err("parsing numeric: %s", csserr_str(e));
                continue;
            }
        } else {
            dcss_warn("Unhandled attribute: `%s`", att->name->data);
        }
    }
    for(size_t i = 0; i < tag->children.len; ++i) {
        apply_css_styles(tag->children.items[i], rootFontSize);
    }
}
