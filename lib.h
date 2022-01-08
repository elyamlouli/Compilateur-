#ifndef LIB_H
#define LIB_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct HashTable HashTable;
typedef struct HashTableBucketRoot HashTableBucketRoot;
typedef struct HashTableBucket HashTableBucket;
typedef struct SymboleTableRoot SymboleTableRoot;
typedef struct SymboleTable SymboleTable;
typedef struct Symbole Symbole;
typedef struct ListSymboles ListSymboles;

typedef struct Code Code;
typedef struct Quad Quad;
typedef struct FunctionsContexts FunctionsContexts;


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

void HashTable_dump(HashTable * hashtable);

struct HashTable {
    size_t size;    // le nombre de buckets
    size_t count;   // nombre total d'éléments
    HashTableBucketRoot * buckets;
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
    size_t temporary;
    SymboleTable * next;
};

SymboleTableRoot * SymboleTableRoot_new();

void SymboleTableRoot_free(SymboleTableRoot * root);

void SymboleTableRoot_dump(SymboleTableRoot * root);

/* Fonctions de manipulation des symboles */

void pushctx(SymboleTableRoot * root);

void popctx(SymboleTableRoot * root);

Symbole * lookup(SymboleTableRoot * root, const char * name);

Symbole * newname(SymboleTableRoot * root, const char * name);

Symbole * newconst(SymboleTableRoot * root, const char * name);

Symbole * newtemp (SymboleTableRoot * root);

Symbole * newfunc(SymboleTableRoot * root, const char * name);

struct Symbole {
    // enum sym_type { CONST_INT, CONST_BOOL, CONST_STRING, CONST_CHAR, VAR_INT, VAR_BOOL, VAR_TAB, NOT_TAB } type;
    enum sym_type { T_NONE, T_INT, T_BOOL, T_STRING, T_CHAR, T_VOID } type;
    enum sym_kind { K_NONE, K_CONST, K_VAR, K_TAB, K_TAB_IDX, K_GLOB, K_FCT, K_ARG } kind;
    char * name;
    union {
        // un truc pour les fonctions
        int32_t int_lit;    // pour les constantes entiere
        int8_t bool_lit;    // pour les constantes boolenne
        char * string_lit;  // pour les constantes string
        char char_lit;      // pour les constantes char
        int tab_size;       // taille du tableau
        Symbole * tab;      // référence au tableau
        ListSymboles *args; // arguments 
    } value;        
    uint32_t offset; // adresse dans le code mips
                    // offset par rapport au SP pour les variables locales
                    // offset par rapport à la déclaration? des variables globales 
};

Symbole * Symbole_new(const char * name);

void Symbole_free(Symbole * symbole);




struct Quad {
  enum quad_kind { 
    OP_LV, OP_GV, OP_NFC, OP_DFC,
    OP_GE, OP_LE, OP_NE, OP_GT, OP_LT,
    OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_MOD,
    OP_INCR, OP_DECR, OP_EQ,
    OP_AND, OP_OR, OP_NOT,
    OP_UMOINS,
    OP_CALL, OP_WS,
    } kind;
  Symbole * sym1;
  Symbole * sym2;
  Symbole * sym3;
};

struct Code {
    unsigned int capacity;
    unsigned int nextquad;
    Quad * quads;
};

struct Code * Code_new();

void Code_free(Code * code);

void gencode(Code * code, enum quad_kind k, Symbole * s1, Symbole * s2, Symbole * s3);


struct ListSymboles {
    size_t size;
    size_t count;
    Symbole ** symboles;
};

struct ListSymboles * ListSymboles_new();

void ListSymboles_free(struct ListSymboles * l);

void ListSymboles_add(struct ListSymboles * l, struct Symbole * symbole);


struct FunctionsContexts {
    size_t size;
    size_t count;
    struct ListSymboles ** lists_sym;
};

FunctionsContexts * FunctionsContexts_new();

void FunctionsContexts_free(struct FunctionsContexts * ctx);

void FunctionsContexts_push(struct FunctionsContexts * ctx);

void FunctionsContexts_pop(struct FunctionsContexts * ctx);

void FunctionsContexts_new_var(struct FunctionsContexts * ctx, Symbole * sym);

void genMIPS(FILE * file, Code * code, SymboleTableRoot * symtable, FunctionsContexts * ctx);

void genMIPS_data(FILE *file, SymboleTableRoot * root, size_t gv_count);



#endif