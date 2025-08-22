#pragma once
#include <stddef.h>
#include <stdbool.h>
typedef struct Atom Atom;
// TODO: store the hash inside the atom
// for MAXIMUM efficiency xD
struct Atom {
    size_t len;
    char data[];
};
typedef struct AtomTableBucket AtomTableBucket;
struct AtomTableBucket {
    AtomTableBucket* next;
    Atom* atom;
};
typedef struct AtomTable {
    struct {
        AtomTableBucket **items;
        size_t len;
    } buckets;
    size_t len;
} AtomTable;
bool atom_table_reserve(AtomTable* map, size_t extra);
// NOTE: does NOT check for duplicates.
// Make sure to call atom_table_get before insert to make sure there is no such atom
bool atom_table_insert(AtomTable* map, Atom* atom);
Atom* atom_table_get(AtomTable* map, const char* data, size_t data_len);
Atom* atom_new(const char* data, size_t n);
Atom* atom_new_cstr(const char* data);
