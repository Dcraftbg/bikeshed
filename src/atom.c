#include "atom.h"
#include <string.h>

#if 0
# define AT_ASSERT(a) (void)(a);
#else
# include <assert.h>
# define AT_ASSERT assert
#endif

#include <stdlib.h>
#define ATOM_TABLE_ALLOC malloc
#define ATOM_TABLE_DEALLOC(ptr, n) free(ptr)

#define ATOM_TABLE_BUCKET_ALLOC   malloc
#define ATOM_TABLE_BUCKET_DEALLOC free
static size_t djb2(const char* str, size_t n) {
    size_t hash = 5381;
    for(size_t i = 0; i < n; ++i) {
        hash = ((hash << 5) + hash) + (unsigned char)str[i];
    }
    return hash;
}
bool atom_table_reserve(AtomTable* map, size_t extra) {
    if(map->len + extra > map->buckets.len) {
        size_t ncap = map->buckets.len*2 + extra;
        AtomTableBucket** newbuckets = ATOM_TABLE_ALLOC(sizeof(*newbuckets)*ncap);
        if(!newbuckets) return false;
        memset(newbuckets, 0, sizeof(*newbuckets) * ncap);
        for(size_t i = 0; i < map->buckets.len; ++i) {
            AtomTableBucket* oldbucket = map->buckets.items[i];
            while(oldbucket) {
                AtomTableBucket* next = oldbucket->next;
                size_t hash = (djb2(oldbucket->atom->data, oldbucket->atom->len)) % ncap;
                AtomTableBucket* newbucket = newbuckets[hash];
                oldbucket->next = newbucket;
                newbuckets[hash] = oldbucket;
                oldbucket = next;
            }
        }
        ATOM_TABLE_DEALLOC(map->buckets.items, map->buckets.cap * sizeof(*map->buckets.items));
        map->buckets.items = newbuckets;
        map->buckets.len = ncap;
    }
    return true;
}
bool atom_table_insert(AtomTable* map, Atom* atom) {
    if(!atom_table_reserve(map, 1)) return false;
    size_t hash = (djb2(atom->data, atom->len)) % map->buckets.len;
    AtomTableBucket* into = map->buckets.items[hash];
    AtomTableBucket* bucket = ATOM_TABLE_BUCKET_ALLOC(sizeof(AtomTableBucket));
    if(!bucket) return false;
    bucket->next = into;
    bucket->atom = atom;
    map->buckets.items[hash] = bucket;
    map->len++;
    return true;
}
Atom* atom_table_get(AtomTable* map, const char* data, size_t data_len) {
    if(map->len == 0) return NULL;
    assert(map->buckets.len > 0);
    size_t hash = djb2(data, data_len) % map->buckets.len;
    AtomTableBucket* bucket = map->buckets.items[hash];
    while(bucket) {
        if(bucket->atom->len == data_len && memcmp(bucket->atom->data, data, data_len) == 0) return bucket->atom;
        bucket = bucket->next;
    }
    return NULL;
}

Atom* atom_new(const char* data, size_t n) {
    Atom* atom = malloc(sizeof(*atom) + n + 1);
    assert(atom && "Just buy more RAM");
    atom->len = n;
    memcpy(atom->data, data, n);
    atom->data[n] = '\0';
    return atom;
}
Atom* atom_new_cstr(const char* data) {
    return atom_new(data, strlen(data));
}
