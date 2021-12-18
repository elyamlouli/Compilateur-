#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "lib.h"

typedef struct HashTable HashTable;
typedef struct HashTableBucket HashTableBucket;
typedef struct SymboleTableRoot SymboleTableRoot;
typedef struct SymboleTable SymboleTable;
typedef struct Symbole Symbole;

#define CHECKMALLOC(op) if (op == NULL) { perror(#op); exit(1); }


HashTable * HashTable_new() {
    HashTable * hashtable = malloc(sizeof(HashTable));
    CHECKMALLOC(hashtable);
    hashtable->size = 8;
    hashtable->count = 0;
    hashtable->bucket = calloc(8, sizeof(HashTableBucket));
    CHECKMALLOC(hashtable->bucket);
    return hashtable;
}

void HashTable_free(HashTable * hashtable){
    for (size_t i = 0; i < hashtable->size; i++) {
        HashTableBucket * bucket = hashtable->bucket[i];
        while (bucket != NULL) {
            HashTableBucket * b = bucket->next;
            Symbole_free(bucket->symbole);
            free(bucket);
            bucket = b;
        }
    }
    free(hashtable);
}

void insert(HashTable * hashtable, Symbole * symbole);

Symbole * get(char * name);



SymboleTableRoot * SymboleTableRoot_new();

void SymboleTableRoot_free(SymboleTableRoot * symtable);

void pushctx(SymboleTableRoot * symtable);

void popctx(SymboleTableRoot * symtable);

Symbole * lookup(SymboleTableRoot * symtable);

void newname(SymboleTableRoot * symtable, char * name);




Symbole * Symbole_new(char * name){
    Symbole * symbole = malloc(sizeof(Symbole));
    CHECKMALLOC(symbole);
    size_t size = strlen(name);
    char * name = malloc(size);
    CHECKMALLOC(name);
    strncpy(symbole->name, name, size);
    symbole->value = 0;
    symbole->type = 0;
    return symbole;
}

void Symbole_free(Symbole * symbole){
    free(symbole->name);
    free(symbole);
}
