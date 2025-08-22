#pragma once
typedef struct HTMLTag HTMLTag;
typedef struct CSSPatternMaps CSSPatternMaps;
void match_css_patterns(HTMLTag* tag, CSSPatternMaps* selector_maps);
