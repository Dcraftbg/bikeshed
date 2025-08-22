#pragma once
#include "atom.h"
typedef AtomTable AtomSet;
bool atom_set_insert(AtomSet* set, Atom* atom);
bool atom_set_get(AtomSet* set, Atom* atom);

