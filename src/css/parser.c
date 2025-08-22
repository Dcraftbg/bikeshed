#include "parser.h"
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include "../darray.h"
#include "../atom.h"
#include <stdlib.h>
#include "../todo.h"
#include "../html.h"

static_assert(CSSERR_COUNT == 5, "Update csserr_strtab");
static const char* csserr_strtab[] = {
    [CSSERR_EOF] = "End of File",
    [CSSERR_INVALID_TAG_NAME] = "Invalid tag name",
    [CSSERR_INVALID_ATTRIBUTE_SYNTAX] = "Invalid attribute syntax",
    [CSSERR_INVALID_ARG_SYNTAX] = "Invalid argument syntax",
};
const char* csserr_str(int err) {
    if(err >= 0) return "OK";
    err = -err;
    if(err >= CSSERR_COUNT) return "Unknown error";
    return csserr_strtab[err];
}
// Skip comments and whitespace
const char* css_skip(const char* content, const char* content_end) {
    for(;;) {
        while(content < content_end && isspace(*content)) content++;
        if(content + 2 < content_end && content[0] == '/' && content[1] == '*') {
            content += 2;
            // FIXME: This is technically invalid but I don't care.
            // Fuck you leather man
            while(content < content_end && (content[0] != '*' || content[1] != '/')) content++;
            if(content >= content_end) break;
            content += 2;
            continue;
        }
        break;
    }
    return content;
}
int css_parse_tag(AtomTable* atom_table, const char* content, const char* content_end, char** end, CSSTag* tag) {
    switch(*content) {
    case '\0': return -CSSERR_EOF;
    case '#': 
        content++;
        tag->kind = CSSTAG_ID;
        break;
    case '.':
        content++;
        tag->kind = CSSTAG_CLASS;
        break;
    default:
        tag->kind = CSSTAG_TAG;
        break;
    }
    const char* name = content;
    while(content < content_end && (isalnum(*content) || *content == '-' || *content == '_')) content++;
    if(name == content) return -CSSERR_INVALID_TAG_NAME;
    tag->name = atom_table_get(atom_table, name, content-name);
    if(!tag->name) {
        tag->name = atom_new(name, content-name);
        assert(atom_table_insert(atom_table, tag->name) && "Just buy more RAM");
    }
    *end = (char*)content;
    return 0;
}
int css_parse_attribute(AtomTable* atom_table, const char* content, const char* content_end, char** end, CSSAttribute* att) {
    const char* name = content;
    while(content < content_end && (isalnum(*content) || *content == '_' || *content == '-')) content++;
    if(name == content) return -CSSERR_INVALID_ATTRIBUTE_SYNTAX;
    att->name = atom_table_get(atom_table, name, content-name);
    if(!att->name) {
        att->name = atom_new(name, content-name);
        assert(atom_table_insert(atom_table, att->name) && "Just buy more RAM");
    }
    content = css_skip(content, content_end);
    if(*content != ':') return -CSSERR_INVALID_ATTRIBUTE_SYNTAX;
    content++;
    size_t oparens = 0;
    for(;;) {
        content = css_skip(content, content_end);
        CSSArg arg;
        arg.value = (char*)content;
        while(content < content_end && ((!isspace(*content) && (content[0] != '/' || content[1] != '*') && *content != ',' && *content != '}' && *content != ';') || oparens > 0)) {
            if(*content == '(') oparens++;
            if(*content == ')' && oparens-- == 0) return -CSSERR_INVALID_ARG_SYNTAX;
            content++;
        }
        if(content == arg.value) return -CSSERR_INVALID_ARG_SYNTAX;
        arg.value_len = content-arg.value;
        content = css_skip(content, content_end);
        da_push(&att->args, arg);
        switch(*content) {
        case '\0':
            return -CSSERR_EOF;
        case ';':
            content++;
            *end = (char*)content;
            return 0;
        case ',':
            content++;
            continue;
        case '}':
            *end = (char*)content;
            return 0;
        }
    }
}
int css_parse_pattern(AtomTable* atom_table, CSSPattern* pattern, const char* css_content, const char* css_content_end, const char** end) {
    int e;
    for(;;) {
        CSSTag tag = { 0 };
        if((e=css_parse_tag(atom_table, css_content, css_content_end, (char**)&css_content, &tag)) < 0) return e;
        da_push(pattern, tag);
        css_content = css_skip(css_content, css_content_end);
        if(!isalnum(*css_content)) break;
    }
    for(size_t i = 0; i < pattern->len/2; ++i) {
        CSSTag temp = pattern->items[i];
        pattern->items[i] = pattern->items[pattern->len-1-i];
        pattern->items[pattern->len-1-i] = temp;
    }
    *end = css_content;
    return 0;
}
int css_parse_patterns(AtomTable* atom_table, CSSPatterns* patterns, const char* css_content, const char* css_content_end, const char** end) {
    int e;
    for(;;) {
        CSSPattern pattern = { 0 };
        if((e=css_parse_pattern(atom_table, &pattern, css_content, css_content_end, &css_content)) < 0) return e;
        da_push(patterns, pattern);
        css_content = css_skip(css_content, css_content_end);
        if(css_content >= css_content_end || *css_content != ',') break;
        css_content++;
        css_content = css_skip(css_content, css_content_end);
    }
    *end = css_content;
    return 0;
}
bool css_match_tag(CSSTag* css_tag, HTMLTag* html_tag) {
    switch(css_tag->kind) {
    case CSSTAG_TAG:
        return html_tag->name == css_tag->name;
    case CSSTAG_ID:
        return html_tag->id == css_tag->name;
    case CSSTAG_CLASS:
        return html_tag->class == css_tag->name;
    default:
        assert(false && "unreachable");
    }
}
bool css_match_pattern(CSSTag* patterns, size_t patterns_count, HTMLTag* html_tag) {
    for(size_t i = 0; i < patterns_count && html_tag; ++i) {
        if(!css_match_tag(&patterns[i], html_tag)) return false;
        html_tag = html_tag->parent;
    }
    return html_tag != NULL;
}
void css_add_attribute(CSSAttributes* attributes, CSSAttribute attribute) {
    for(size_t i = 0; i < attributes->len; ++i) {
        if(attributes->items[i].name == attribute.name) {
            // TODO: cleanup old attribute if necessary
            attributes->items[i] = attribute;
            return;
        }
    }
    da_push(attributes, attribute);
}

