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
typedef struct Context Context;
typedef struct FunctionsContexts FunctionsContexts;
typedef struct ListGoto ListGoto;


/* Section fontion utils decaf */
int op_is_int(Symbole * sym1, Symbole * sym2);

int op_is_bool(Symbole * sym1, Symbole * sym2);


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
    SymboleTable * poped;
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

Symbole * newquadsym(SymboleTableRoot * root);

struct Symbole {
    // enum sym_type { CONST_INT, CONST_BOOL, CONST_STRING, CONST_CHAR, VAR_INT, VAR_BOOL, VAR_TAB, NOT_TAB } type;
    enum sym_type { T_NONE, T_INT, T_BOOL, T_STRING, T_CHAR, T_VOID } type;
    enum sym_kind { K_NONE, K_CONST, K_VAR, K_TAB, K_TAB_IDX, K_GLOB, K_FCT, K_QUAD} kind;
    char * name;
    union {
        // un truc pour les fonctions
        int32_t int_lit;    // pour les constantes entiere
        int8_t bool_lit;    // pour les constantes boolenne
        char * string_lit;  // pour les constantes string
        char char_lit;      // pour les constantes char
        int tab_size;       // taille du tableau
        Symbole * tab[2];   // référence au tableau + référence indice
        ListSymboles *args; // arguments 
        size_t idx_quad;     // quad idx
    } value;        
    uint32_t offset; // adresse dans le code mips
                    // offset par rapport au SP pour les variables locales
                    // offset par rapport à la déclaration? des variables globales 
};

Symbole * Symbole_new(const char * name);

void Symbole_free(Symbole * symbole);


/* section pour les Quads */

struct Quad {
  enum quad_kind { 
    OP_LV, OP_GV, OP_NFC, OP_DFC,
    OP_GE, OP_LE, OP_NE, OP_GT, OP_LT, OP_EQ,
    OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_MOD,
    OP_INCR, OP_DECR, OP_ASSIGN,
    OP_AND, OP_OR, OP_NOT,
    OP_UMOINS,
    OP_CALL, OP_WS, OP_WI, OP_RI, OP_WB,
    OP_PUSH,
    OP_GOTO, OP_GOTO_IF, OP_GOTO_FOR,
    } kind;
  Symbole * sym1;
  Symbole * sym2;
  Symbole * sym3;
  int8_t label;
};

struct Code {
    size_t capacity;
    size_t nextquad;
    Quad * quads;
};

struct Code * Code_new();

void Code_free(Code * code);

void gencode(Code * code, enum quad_kind k, Symbole * s1, Symbole * s2, Symbole * s3);



/* section pour les Goto */

struct ListGoto {
    size_t size;
    size_t count;
    size_t * quads_idx;
};

ListGoto * ListGoto_new();

void ListGoto_free(ListGoto * lg);

void ListGoto_add(ListGoto * lg, size_t quad_idx);

ListGoto * ListGoto_concat(ListGoto * lg1,  ListGoto * lg2);

void ListGoto_complete(Code * code, ListGoto * lg, size_t quad_idx);



/* section pour les listes de symboles */

struct ListSymboles {
    size_t size;
    size_t count;
    Symbole ** symboles;
};

struct ListSymboles * ListSymboles_new();

void ListSymboles_free(struct ListSymboles * l);

void ListSymboles_add(struct ListSymboles * l, struct Symbole * symbole);



/* section pour gérer le contexte */

struct Context {
    Symbole * ret;
    ListSymboles * list_sym;
};

struct FunctionsContexts {
    size_t size;
    size_t count;
    Context * list_ctx; // une liste de contexts
};


FunctionsContexts * FunctionsContexts_new();

void FunctionsContexts_free(struct FunctionsContexts * ctx);

void FunctionsContexts_push(struct FunctionsContexts * ctx);

void FunctionsContexts_pop(struct FunctionsContexts * ctx);

void FunctionsContexts_new_var(struct FunctionsContexts * ctx, Symbole * sym);


/* section pour la génération de code MIPS */

void genMIPS(FILE * file, Code * code, SymboleTableRoot * symtable, FunctionsContexts * ctx);

void genMIPS_data(FILE *file, SymboleTableRoot * root, size_t gv_offset);

void genMIPS_inst_load(FILE * file, const char * reg, Symbole * sym);

void genMIPS_inst_store(FILE * file, const char * reg, Symbole * sym);

#endif