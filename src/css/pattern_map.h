#pragma once
#include "parser.h"
typedef struct CSSPatternMapBucket CSSPatternMapBucket;
struct CSSPatternMapBucket {
    CSSPatternMapBucket* next;
    // Essentially equivilent to pattern.items[0].name. We just cache it cuz its not changed
    // anyway so who cares
    Atom* name;
    CSSPatterns patterns;
};
typedef struct {
    struct {
        CSSPatternMapBucket** items;
        size_t len;
    } buckets;
    size_t len;
} CSSPatternMap;
typedef struct CSSPatternMaps {
    CSSPatternMap maps[CSSTAG_COUNT];
} CSSPatternMaps;

bool css_pattern_map_reserve(CSSPatternMap* map, size_t extra);
bool css_pattern_map_insert(CSSPatternMap* map, Atom* name, CSSPatterns patterns);
CSSPatterns* css_pattern_map_get(CSSPatternMap* map, Atom* name);

CSSPatterns* css_pattern_map_get_or_insert_empty(CSSPatternMap* map, Atom* name);
