#include "../css/pattern_map.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define CSS_PATTERN_MAP_ALLOC malloc
#define CSS_PATTERN_MAP_DEALLOC(ptr, n) free(ptr)

#define CSS_PATTERN_MAP_BUCKET_ALLOC   malloc
#define CSS_PATTERN_MAP_BUCKET_DEALLOC free
bool css_pattern_map_reserve(CSSPatternMap* map, size_t extra) {
    if(map->len + extra > map->buckets.len) {
        size_t ncap = map->buckets.len*2 + extra;
        CSSPatternMapBucket** newbuckets = CSS_PATTERN_MAP_ALLOC(sizeof(*newbuckets)*ncap);
        if(!newbuckets) return false;
        memset(newbuckets, 0, sizeof(*newbuckets) * ncap);
        for(size_t i = 0; i < map->buckets.len; ++i) {
            CSSPatternMapBucket* oldbucket = map->buckets.items[i];
            while(oldbucket) {
                CSSPatternMapBucket* next = oldbucket->next;
                size_t hash = ((size_t)oldbucket->name) % ncap;
                CSSPatternMapBucket* newbucket = newbuckets[hash];
                oldbucket->next = newbucket;
                newbuckets[hash] = oldbucket;
                oldbucket = next;
            }
        }
        CSS_PATTERN_MAP_DEALLOC(map->buckets.items, map->buckets.cap * sizeof(*map->buckets.items));
        map->buckets.items = newbuckets;
        map->buckets.len = ncap;
    }
    return true;
}
bool css_pattern_map_insert(CSSPatternMap* map, Atom* name, CSSPatterns patterns) {
    if(!css_pattern_map_reserve(map, 1)) return false;
    size_t hash = ((size_t)name) % map->buckets.len;
    CSSPatternMapBucket* into = map->buckets.items[hash];
    CSSPatternMapBucket* bucket = CSS_PATTERN_MAP_BUCKET_ALLOC(sizeof(CSSPatternMapBucket));
    if(!bucket) return false;
    bucket->next = into;
    bucket->name = name;
    bucket->patterns = patterns;
    map->buckets.items[hash] = bucket;
    map->len++;
    return true;
}
CSSPatterns* css_pattern_map_get(CSSPatternMap* map, Atom* name) {
    if(map->len == 0) return NULL;
    assert(map->buckets.len > 0);
    size_t hash = ((size_t)name) % map->buckets.len;
    CSSPatternMapBucket* bucket = map->buckets.items[hash];
    while(bucket) {
        if(bucket->name == name) return &bucket->patterns;
        bucket = bucket->next;
    }
    return NULL;
}
CSSPatterns* css_pattern_map_get_or_insert_empty(CSSPatternMap* map, Atom* name) {
    CSSPatterns* patterns = css_pattern_map_get(map, name);
    if(patterns) return patterns;
    css_pattern_map_insert(map, name, (CSSPatterns){0});
    return css_pattern_map_get(map, name);
}
