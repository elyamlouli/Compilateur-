#ifndef LIB_H
#define LIB_H

#include <stdint.h>
#include <stdlib.h>

typedef struct HashTable HashTable;
typedef struct HashTableBucketRoot HashTableBucketRoot;
typedef struct HashTableBucket HashTableBucket;
typedef struct SymboleTableRoot SymboleTableRoot;
typedef struct SymboleTable SymboleTable;
typedef struct Symbole Symbole;


/* Section pour la HashTable */

struct HashTableBucketRoot {
    HashTableBucket * next;
};

struct HashTableBucket {
    Symbole * symbole;
    HashTableBucket * next;
};

HashTableBucket * HashTableBucket_new(Symbole * symbole);

HashTableBucket * HashTableBucket_free(HashTableBucket *bucket);

struct HashTable {
    size_t size;
    size_t count;
    HashTableBucket ** buckets;
} ;

HashTable * HashTable_new();

void HashTable_free(HashTable * hashtable);

int HashTable_insert(HashTable * hashtable, Symbole * symbole);

Symbole * HashTable_get(HashTable * hashtable, const char * name);


/* Section pour la SymboleTable */

struct SymboleTable {
    HashTable * table; 
    SymboleTable * next;
};

SymboleTable * SymboleTable_new();

SymboleTable * SymboleTable_free(SymboleTable * symtable);


/* Section pour la SymboleTableRoot */

struct SymboleTableRoot {
    SymboleTable * next;
};

SymboleTableRoot * SymboleTableRoot_new();

void SymboleTableRoot_free(SymboleTableRoot * root);


/* Fonctions de manipulation des symboles */

void pushctx(SymboleTableRoot * root);

void popctx(SymboleTableRoot * root);

Symbole * lookup(SymboleTableRoot * root, const char * name);

int newname(SymboleTableRoot * root, const char * name);


struct Symbole {
    char * name;
    int32_t value;
    int8_t type;
};

Symbole * Symbole_new(const char * name);

void Symbole_free(Symbole * symbole);


#endif