#ifndef LIB_H
#define LIB_H

#include <stdint.h>
#include <stdlib.h>

typedef struct HashTable HashTable;
typedef struct HashTableBucket HashTableBucket;
typedef struct SymboleTableRoot SymboleTableRoot;
typedef struct SymboleTable SymboleTable;
typedef struct Symbole Symbole;



struct HashTableBucket {
    Symbole * symbole;
    HashTableBucket * next;
};

struct HashTable {
    size_t size;
    size_t count;
    HashTableBucket ** bucket;
} ;

HashTable * HashTable_new();

void HashTable_free(HashTable * hashtable);

void insert(HashTable * hashtable, Symbole * symbole);

Symbole * get(char * name);


struct SymboleTable {
    HashTable * table; 
    SymboleTable * next;
};

struct SymboleTableRoot {
    SymboleTable * next;
};


SymboleTableRoot * SymboleTableRoot_new();

void SymboleTableRoot_free(SymboleTableRoot * symtable);

void pushctx(SymboleTableRoot * symtable);

void popctx(SymboleTableRoot * symtable);

Symbole * lookup(SymboleTableRoot * symtable);

void newname(SymboleTableRoot * symtable, char * name);


struct Symbole {
    char * name;
    int32_t value;
    int8_t type;
};

Symbole * Symbole_new(char * name);

void Symbole_free(Symbole * symbole);


#endif