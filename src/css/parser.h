#pragma once
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

enum {
    CSSERR_EOF=1,
    CSSERR_INVALID_TAG_NAME,
    CSSERR_INVALID_ATTRIBUTE_SYNTAX,
    CSSERR_INVALID_ARG_SYNTAX,

    CSSERR_COUNT
};
enum {
    // #foo
    CSSTAG_ID,
    // .foo
    CSSTAG_CLASS,
    // foo
    CSSTAG_TAG,
    CSSTAG_COUNT,
};
typedef struct AtomTable AtomTable;
typedef struct Atom Atom;
typedef uint32_t CSSTagKind;
typedef struct {
    Atom* name;
    CSSTagKind kind;
} CSSTag;
typedef struct {
    char* value;
    size_t value_len;
} CSSArg;
typedef struct {
    Atom* name;
    struct {
        CSSArg* items;
        size_t len, cap;
    } args;
} CSSAttribute;
// TODO: convert to hashmap :(
typedef struct {
    CSSAttribute* items;
    size_t len, cap;
} CSSAttributes;
void css_add_attribute(CSSAttributes* attributes, CSSAttribute attribute);
typedef struct {
    CSSTag* items;
    size_t len, cap;
    // TODO: CSSTag inline_buffer[1];
    CSSAttributes attributes;
} CSSPattern;
typedef struct {
    CSSPattern* items;
    size_t len, cap;
} CSSPatterns;
// Skip spaces and comments
const char* css_skip(const char* content, const char* content_end);
const char* csserr_str(int err);
int css_parse_tag(AtomTable* atom_table, const char* content, const char* content_end, char** end, CSSTag* tag);
int css_parse_attribute(AtomTable* atom_table, const char* content, const char* content_end, char** end, CSSAttribute* att);
int css_parse_pattern(AtomTable* atom_table, CSSPattern* pattern, const char* css_content, const char* css_content_end, const char** end);
int css_parse_patterns(AtomTable* atom_table, CSSPatterns* patterns, const char* css_content, const char* css_content_end, const char** end);
typedef struct HTMLTag HTMLTag;
bool css_match_tag(CSSTag* css_tag, HTMLTag* html_tag);
bool css_match_pattern(CSSTag* patterns, size_t patterns_count, HTMLTag* html_tag);
