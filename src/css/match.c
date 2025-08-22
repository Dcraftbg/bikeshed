#include "../html.h"
#include "pattern_map.h"
#include "../darray.h"

void match_css_patterns(HTMLTag* tag, CSSPatternMaps* selector_maps) {
    CSSPatterns* patterns = css_pattern_map_get(&selector_maps->maps[CSSTAG_TAG], tag->name);
    if(patterns) {
        for(size_t i = 0; i < patterns->len; ++i) {
            CSSPattern* pattern = &patterns->items[i];
            if(css_match_pattern(pattern->items, pattern->len, tag)) {
                for(size_t j = 0; j < pattern->attributes.len; ++j) {
                    css_add_attribute(&tag->css_attribs, pattern->attributes.items[j]);
                }
            }
        }
    }
    patterns = css_pattern_map_get(&selector_maps->maps[CSSTAG_ID], tag->id);
    if(patterns) {
        for(size_t i = 0; i < patterns->len; ++i) {
            CSSPattern* pattern = &patterns->items[i];
            if(css_match_pattern(pattern->items, pattern->len, tag)) {
                for(size_t j = 0; j < pattern->attributes.len; ++j) {
                    css_add_attribute(&tag->css_attribs, pattern->attributes.items[j]);
                }
            }
        }
    }
    patterns = css_pattern_map_get(&selector_maps->maps[CSSTAG_CLASS], tag->class);
    if(patterns) {
        for(size_t i = 0; i < patterns->len; ++i) {
            CSSPattern* pattern = &patterns->items[i];
            if(css_match_pattern(pattern->items, pattern->len, tag)) {
                for(size_t j = 0; j < pattern->attributes.len; ++j) {
                    css_add_attribute(&tag->css_attribs, pattern->attributes.items[j]);
                }
            }
        }
    }
    for(size_t i = 0; i < tag->children.len; ++i) {
        match_css_patterns(tag->children.items[i], selector_maps);
    }
}
