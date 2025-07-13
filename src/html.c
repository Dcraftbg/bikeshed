#include "html.h"
#include "assert.h"
#include <stdio.h>
#include <ctype.h>
#include <atom.h>
#include <darray.h>
#include <string.h>

static_assert(HTMLERR_COUNT == 7, "Update htmlerr_strtab");
static const char* htmlerr_strtab[] = {
    [HTMLERR_TODO] = "Unimplemented",
    [HTMLERR_EOF]  = "End of File",
    [HTMLERR_INVALID_TAG] = "Invalid tag format",
    [HTMLERR_INVALID_TAG_NAME] = "Invalid tag name",
    [HTMLERR_INVALID_ATTRIBUTE]  = "Invalid attribute format",
    [HTMLERR_INVALID_ATTRIBUTE_NAME] = "Invalid attribute name",
};
const char* htmlerr_str(int err) {
    if(err >= 0) return "OK";
    err = -err;
    if(err >= HTMLERR_COUNT) return "Unknown error";
    return htmlerr_strtab[err];
}
int html_parse_attribute(const char* content, HTMLAttribute* att, const char** end) {
    att->key = (char*)content;
    while (isalnum(*content) || *content == '_' || *content == '-' || *content == ':')
        content++;
    if(att->key == content) return -HTMLERR_INVALID_ATTRIBUTE_NAME;
    att->key_len = content - att->key;
    while(isspace(*content)) content++;
    if (*content != '=') {
        // it has no value
        *end = content;
        return 0;
    }
    content++;
    while (isspace(*content)) content++;
    char quote = *content;
    if (*content != '"' && *content != '\'') {
        att->value = (char*)content;
        while(isalnum(*content)) content++;
        if(content == att->value) return -HTMLERR_INVALID_ATTRIBUTE;
        att->value_len = content - att->value;
        *end = content;
        return 0;
    } 
    content++;
    att->value = (char*)content;
    while(*content && *content != quote) content++;
    if(*content != quote) return -HTMLERR_EOF;
    att->value_len = content - att->value;
    content++;
    *end = content;
    return 0;
}

int html_parse_next_tag(AtomTable* atom_table, const char* content, HTMLTag* tag, char** end) {
    if(*content == '<') {
        content++;
        const char* name = content;
        while(isalnum(*content)) content++;
        if(content == name) return -HTMLERR_INVALID_TAG_NAME;
        tag->name = atom_table_get(atom_table, name, content-name);
        if(!tag->name) {
            tag->name = atom_new(name, content - name);
            assert(atom_table_insert(atom_table, tag->name) && "Just buy more RAM");
        }
        while (*content && *content != '>' && *content != '/') {
            while(isspace(*content)) content++;
            int e;
            HTMLAttribute *att = (HTMLAttribute*) malloc(sizeof(HTMLAttribute));
            assert(att && "Just buy more RAM");
            memset(att, 0, sizeof(*att));
            if ((e=html_parse_attribute(content, att, &content)) < 0) {
                free(att);
                return e;
            }
            da_push(&tag->attributes, att);
        }
        if (*content == '/') {
            if (content[1] != '>') return -HTMLERR_INVALID_TAG;
            content += 2;
            *end = (char*) content;
            tag->self_closing = true;
            return 0;
        }
        tag->self_closing = false;
        content++;
        *end = (char*)content;
        return 0;
    }
    if(*content == '\0') return -HTMLERR_EOF;
    tag->str_content = content;
    while(*content != '<' && *content) content++;
    tag->str_content_len = content - tag->str_content;
    *end = (char*)content;
    return 0;
}

void dump_html_tag(HTMLTag* tag, size_t indent) {
    if(tag->name) {
        if(tag->name->len == 5 && strncmp(tag->name->data, "style", 5) == 0) return;
        printf("%*s<%.*s>\n", (int)indent, "", (int)tag->name->len, tag->name->data);
        for(size_t i = 0; i < tag->children.len; ++i) {
            dump_html_tag(tag->children.items[i], indent + 4);
        }
        printf("%*s</%.*s>\n", (int)indent, "", (int)tag->name->len, tag->name->data);
    } else {
        printf("%*s", (int)indent, "");
        for(size_t i = 0; i < tag->str_content_len; ++i) {
            char c = tag->str_content[i];
            if(isgraph(c) || c == ' ') printf("%c", c);
            else printf("\\x%02X", c);
        }
        printf("\n");
    }
}
