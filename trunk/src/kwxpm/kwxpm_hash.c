/* KiWin - A small GUI for the embedded system
 * Copyright (C) <2007>  Wei Hu <wei.hu.tw@gmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* must be called before allocating any atom. */
bool
xpmHashTableInit(xpmHashTable *table)
{
  xpmHashAtom *p = NULL;
  xpmHashAtom *atomTable = NULL;
  
  table->size = INITIAL_HASH_SIZE;
  table->limit = table->size / 3;
  table->used = 0;
  
  atomTable = (xpmHashAtom *)malloc(table->size * sizeof(*atomTable));
  if (atomTable == NULL) {
    LOG(("%s\n", "<Xpm Error> malloc() failed."));
    return false;
  }
  
  for (p = atomTable + table->size; p > atomTable;) {
    *--p = NULL;
  }
  
  table->atomTable = atomTable;
  return true;
}

/* frees a hashtable and all the stored atoms. */
void
xpmHashTableFree(xpmHashTable *table)
{
  xpmHashAtom *p = NULL;
  xpmHashAtom *atomTable = table->atomTable;
  
  if (!atomTable) {
    return;
  }
  
  for (p = atomTable + table->size; p > atomTable;) {
    if (*--p) {
      free(*p);
    }
  }
  
  free(atomTable);
  table->atomTable = NULL;
}

/* xpmHashSlot gives the slot (pointer to xpmHashAtom) of a name
 * (slot points to NULL if it is not defined)
 *
 */
xpmHashAtom*
xpmHashSlot(xpmHashTable *table, char *s)
{
  xpmHashAtom *atomTable = table->atomTable;
  unsigned int hash;
  xpmHashAtom *p;
  char *hp = s;
  char *ns;
  
  hash = 0;
  while (*hp) {			/* computes hash function */
    HASH_FUNCTION
      }
  p = atomTable + hash % table->size;
  while (*p) {
    ns = (*p)->name;
    if (ns[0] == s[0] && strcmp(ns, s) == 0) {
      break;
    }
    p--;
    if (p < atomTable) {
      p = atomTable + table->size - 1;
    }
  }
  return p;
}

static int
HashTableGrows(xpmHashTable *table)
{
  xpmHashAtom *atomTable = table->atomTable;
  int size = table->size;
  xpmHashAtom *t, *p;
  int i;
  int oldSize = size;
  
  t = atomTable;
  HASH_TABLE_GROWS
    table->size = size;
  table->limit = size / 3;
  atomTable = (xpmHashAtom *) malloc(size * sizeof(*atomTable));
  if (atomTable == NULL) {
    return false;
  }
  table->atomTable = atomTable;
  for (p = atomTable + size; p > atomTable;) {
    *--p = NULL;
  }
  
  for (i = 0, p = t; i < oldSize; i++, p++) {
    if (*p) {
      xpmHashAtom *ps = xpmHashSlot(table, (*p)->name);
      *ps = *p;
    }
  }
  
  free(t);
  return true;
}

/* makes an atom */
static xpmHashAtom
AtomMake(char *name, void *data)
{
  xpmHashAtom object = (xpmHashAtom)malloc(sizeof(struct _xpmHashAtom));
  
  if (object != NULL) {
    object->name = name;
    object->data = data;
  }
  
  return object;
}

/* xpmHashIntern(table, name, data)
 * an xpmHashAtom is created if name doesn't exist, with the given data.
 */
bool
xpmHashIntern(xpmHashTable *table, char *tag, void *data)
{
  xpmHashAtom *slot;
  
  if (!*(slot = xpmHashSlot(table, tag))) {
    /* undefined, make a new atom with the given data */
    if (!(*slot = AtomMake(tag, data)))
      return false;
    if (table->used >= table->limit) {
      bool result;
      
      if ((result = HashTableGrows(table)) != true) {
	return false;
      }
      table->used++;
      return true;
    }
    table->used++;
  }
  return true;
}
