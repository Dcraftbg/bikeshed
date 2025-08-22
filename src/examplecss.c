#include <stdio.h>
#include "fileutils.h"
#include <string.h>
#include "darray.h"
#include <assert.h>
#include <ctype.h>
#include "css/parser.h"
#include "atom.h"

#define STRINGIFY0(x) # x
#define STRINGIFY1(x) STRINGIFY0(x)
#define todof(...) (fprintf(stderr, "TODO " __FILE__ ":" STRINGIFY1(__LINE__) ":" __VA_ARGS__), abort())

int cssmain(void) {
    const char* example_path = "examples/sample.css";
    size_t content_size;
    char* content_data = (char*)read_entire_file(example_path, &content_size);
    if(!content_data) return 1;
    const char* content = content_data;
    const char* content_end = content_data + strlen(content_data);
    AtomTable atom_table = { 0 };
    for(;;) {
        content = css_skip(content, content_end);
        if(*content == '\0') break;
        CSSTag tag;
        int e;
        if((e = css_parse_tag(&atom_table, content, content_end, (char**)&content, &tag)) < 0) {
            fprintf(stderr, "ERROR %s\n", csserr_str(e));
            return 1;
        }
        content = css_skip(content, content_end);
        if(*content == ',') todof("Implement coma separated tags");
        else if(isalnum(*content)) todof("Implement space separated tags (patterns)");
        else if(*content != '{') { 
            fprintf(stderr, "Fok you leather man: `%c`\n", *content);
            return 1;
        }
        content++;
        printf("%s {\n", tag.name->data);
        for(;;) {
            content = css_skip(content, content_end);
            if(*content == '}') {
                content++;
                break;
            }
            CSSAttribute attr = { 0 };
            e = css_parse_attribute(&atom_table, content, content_end, (char**)&content, &attr);
            if(e < 0) {
                fprintf(stderr, "ERROR %s\n", csserr_str(e));
                return 1;
            }
            printf("    %s:", attr.name->data);
            for(size_t i = 0; i < attr.args.len; ++i) {
                printf(" %.*s", (int)attr.args.items[i].value_len, attr.args.items[i].value);
            }
            printf(";\n");
            // fprintf(stderr, "Attribute: %s\n", attr.name->data);
        }
        printf("}\n");
    }
    return 0;
}
#if 0
// TODO: update this shit to the newest standard
#include "cssparser.h"
int cssmain(){
    const char* example_path = "examples/sample.css";
    size_t content_size;
    char* content_data = (char*)read_entire_file(example_path, &content_size);
    char* content = content_data;
    remove_carrige_return(content);

    CSSNodes cssNodes = {0};
    int result = parse_css_file(content,&cssNodes);
    if(result < 0) {
        printf("CSSERR: %s\n", csserr_str(result));
        return 1;
    }

    printf("Parsed: %zu", cssNodes.len);
    for(size_t i = 0; i < cssNodes.len; i++){
        CSSNode* node = &cssNodes.items[i];
        printf("\n----\"%.*s\"-----\n", (int)node->name_len, node->name_content);
        if(node->attrs.len > 0) printf("attrs:\n");
        for(size_t j = 0; j < node->attrs.len; j++){
            CSSAttribute* attr = &node->attrs.items[j];
            printf("name: %.*s, values: ", (int)attr->name_len, attr->name_content);
            for(size_t m = 0; m < attr->values.len; m++){
                CSSAttributeValue* attrVal = &attr->values.items[m];
                printf("%.*s ", (int)attrVal->value_len, attrVal->value_content);
            }
            printf("\n");
        }
    }

    return 0;
}
#endif
