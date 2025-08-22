#include "fixup.h"
#include "html.h"
#include "darray.h"
#include <string.h>
#include <assert.h>

void fixup_tree(HTMLTag* tag){
    /* 
        if we encounter blocks inside we need to get them out of children put them
        after this tag and if there are any spare inline blocks left then create clone
        of this tag that has rest of spare tags 
    */
    for(size_t i = 0; i < tag->children.len; i++){
        HTMLTag* child = tag->children.items[i];
        fixup_tree(child);
        
        HTMLTag* clone = NULL;
        if(child->display == CSSDISPLAY_INLINE){
            for(size_t j = 0; j < child->children.len; j++){
                if(child->children.items[j]->display == CSSDISPLAY_BLOCK){
                    if(j == 0){
                        da_insert(&tag->children, i, child->children.items[j]);
                        memmove(&child->children.items[0], &child->children.items[1], (--child->children.len)*sizeof(*child->children.items));
                    }else if(j < child->children.len-1){ // create clone
                        clone = malloc(sizeof(HTMLTag));
                        memcpy(clone, child, sizeof(HTMLTag));
                        clone->children.items = NULL;
                        clone->children.cap = 0;
                        clone->children.len = 0;
                        
                        for(size_t m = j+1; m < child->children.len; m++){
                            da_push(&clone->children,child->children.items[m]);
                        }
                        child->children.len = j;
                        da_insert(&tag->children, i+1, child->children.items[j]);
                    }else{
                        da_insert(&tag->children, i+1, child->children.items[j]);
                        child->children.len--;
                    }
                    break;
                }
            }
            if(clone) da_insert(&tag->children, i+2, clone);
        }
    }
}
