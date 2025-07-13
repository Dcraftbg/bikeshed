#pragma once
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
enum {
    HTMLERR_TODO=1,
    HTMLERR_EOF,
    HTMLERR_INVALID_TAG,
    HTMLERR_INVALID_TAG_NAME,
    HTMLERR_INVALID_ATTRIBUTE,
    HTMLERR_INVALID_ATTRIBUTE_NAME,
    HTMLERR_COUNT,
};
typedef struct Atom Atom;
typedef struct HTMLAttribute HTMLAttribute;
typedef struct {
    HTMLAttribute** items;
    size_t len, cap;
} HTMLAttributes;
struct HTMLAttribute {
    char* key;
    size_t key_len;
    char* value; // value is NULL if it has no value
    size_t value_len;
};
typedef struct HTMLTag HTMLTag;
typedef struct {
    HTMLTag** items;
    size_t len, cap;
} HTMLTags;
// TODO: refactor this out to CSS
// and some sort of CSSStyle
enum {
    CSSDISPLAY_INLINE,
    CSSDISPLAY_INLINE_BLOCK,
    CSSDISPLAY_BLOCK,

    CSSDISPLAY_COUNT
};
typedef uint32_t CSSDisplay;
#include <css/parser.h>
#include <css/parse_values.h>
struct HTMLTag {
    HTMLTag* parent;
    Atom* name;
    Atom* class;
    Atom* id;
    HTMLTags children;
    const char* str_content;
    size_t str_content_len;
    HTMLAttributes attributes;
    // --- CSS ---
    CSSAttributes css_attribs;
    CSSDisplay display;
    float fontSize;
    CSSColor background_color;
    CSSColor color;
    // --- CSS ---
    bool self_closing;
    // Box of the tag 
    size_t x, y;
    size_t width, height;
};

typedef struct AtomTable AtomTable;
const char* htmlerr_str(int err);
int html_parse_attribute(const char* content, HTMLAttribute* att, const char** end);
int html_parse_next_tag(AtomTable* atom_table, const char* content, HTMLTag* tag, char** end);
void dump_html_tag(HTMLTag* tag, size_t indent);
