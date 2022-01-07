#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "lib.h"

typedef struct HashTable HashTable;
typedef struct HashTableBucketRoot HashTableBucketRoot;
typedef struct HashTableBucket HashTableBucket;
typedef struct SymboleTableRoot SymboleTableRoot;
typedef struct SymboleTable SymboleTable;
typedef struct Symbole Symbole;

typedef struct Code Code;
typedef struct Quad Quad;

#define CHECKMALLOC(op) if (op == NULL) { perror(#op); exit(1); }


HashTableBucket * HashTableBucket_new(Symbole * symbole) {
    HashTableBucket *bucket = malloc(sizeof(HashTableBucket));
    CHECKMALLOC(bucket);
    bucket->symbole = symbole;
    bucket->next = NULL;
    return bucket;
}

HashTableBucket * HashTableBucket_free(HashTableBucket *bucket) {
    HashTableBucket * next = bucket->next;
    Symbole_free(bucket->symbole);
    free(bucket);
    return next;
}

HashTable * HashTable_new() {
    HashTable * hashtable = malloc(sizeof(HashTable));
    CHECKMALLOC(hashtable);
    hashtable->size = 8;
    hashtable->count = 0;
    hashtable->buckets = calloc(hashtable->size, sizeof(HashTableBucketRoot*));
    CHECKMALLOC(hashtable->buckets);
    return hashtable;
}

void HashTable_free(HashTable * hashtable){
    for (size_t i = 0; i < hashtable->size; i++) {
        HashTableBucket * bucket = hashtable->buckets[i].next; 
        while (bucket != NULL) {
            bucket = HashTableBucket_free(bucket);
        }
    }
    free(hashtable->buckets);
    free(hashtable);
}


#define A 54059 /* a prime */
#define B 76963 /* another prime */
#define C 86969 /* yet another prime */
#define FIRSTH 37 /* also prime */
size_t hash_str(const char* s) {
   size_t h = FIRSTH;
   while (*s) {
        h = (h * A) ^ (s[0] * B);
        s++;
   }
   return h; // or return h % C;
}


// TODO : dynamic size 
int HashTable_insert(HashTable * hashtable, Symbole * symbole) {
    size_t id = hash_str(symbole->name) % hashtable->size;

    HashTableBucket * bucket = hashtable->buckets[id].next;
    if (bucket == NULL) {
        hashtable->buckets[id].next = HashTableBucket_new(symbole);
        hashtable->count++;
        return 0;
    }

    HashTableBucket * bucket_prec;
    while (bucket != NULL ) {
        if (strcmp(bucket->symbole->name, symbole->name) == 0) {
            return -1; // name déjà présent 
        }
        bucket_prec = bucket;
        bucket = bucket->next;
    }

    bucket_prec->next = HashTableBucket_new(symbole);
    hashtable->count++;
    return 0;
}

Symbole * HashTable_get(HashTable * hashtable, const char * name) {
    size_t id = hash_str(name) % hashtable->size;
    HashTableBucket *bucket = hashtable->buckets[id].next;
    while (bucket != NULL ) {
        if (strcmp(bucket->symbole->name, name) == 0) {
            return bucket->symbole;
        }
        bucket = bucket->next;
    }
    return NULL;
}

void HashTable_dump(HashTable * hashtable) {
    for (size_t i = 0; i < hashtable->size; i++) {
        HashTableBucket *bucket = hashtable->buckets[i].next;
        while (bucket != NULL) {
            printf("%s\n", bucket->symbole->name);
            bucket = bucket->next;
        }
    }
}


SymboleTable * SymboleTable_new() {
    SymboleTable * symtable = malloc(sizeof(SymboleTable));
    CHECKMALLOC(symtable);
    
    symtable->table = HashTable_new();
    symtable->next = NULL;

    return symtable;
}

SymboleTable * SymboleTable_free(SymboleTable * symtable) {
    SymboleTable * next = symtable->next;
    HashTable_free(symtable->table);
    free(symtable);
    return next;
}


SymboleTableRoot * SymboleTableRoot_new() {
    SymboleTableRoot * root = malloc(sizeof(SymboleTableRoot));
    CHECKMALLOC(root);

    root->next = SymboleTable_new();
    root->temporary = 0;

    return root;
}

void SymboleTableRoot_free(SymboleTableRoot * root) {
    SymboleTable * symtable = root->next;
    while (symtable != NULL) {
        symtable = SymboleTable_free(symtable);
    }
    free(root);
}

void SymboleTableRoot_dump(SymboleTableRoot * root) {
    SymboleTable * symtable = root->next;
    while (symtable != NULL) {
        HashTable_dump(symtable->table);
        printf("\n");
        symtable = symtable->next;
    }
}

void pushctx(SymboleTableRoot * root) {
    SymboleTable * new_symtable = SymboleTable_new();
    new_symtable->next = root->next;
    root->next = new_symtable;
}

void popctx(SymboleTableRoot * root) {
    SymboleTable * symtable = root->next;
    root->next = symtable->next;
    SymboleTable_free(symtable);
}

Symbole * lookup(SymboleTableRoot * root, const char * name ) {
    SymboleTable * symtable = root->next;
    while (symtable != NULL) {
        Symbole * symbole = HashTable_get(symtable->table, name);
        if (symbole != NULL) {
            return symbole;
        }
        symtable = symtable->next;
    }
    return NULL;
}

Symbole * newname(SymboleTableRoot * root, const char * name) {
    Symbole * symbole = Symbole_new(name);
    int res = HashTable_insert(root->next->table, symbole);
    if (res == -1) { // name déjà présent
        fprintf(stderr, "newname failed : \"%s\" already taken\n", name);
        Symbole_free(symbole);
        exit(1);
    }
    return symbole;
}


// "sqdqdqsf"
// -C\t
// -I10000000
// -B1/0 char [15]
Symbole * newconst(SymboleTableRoot * root, const char * name) {
    SymboleTable * symtable = root->next;
    while (symtable->next != NULL) {
        symtable = symtable->next;
    }

    Symbole * symbole = HashTable_get(symtable->table, name);
    if (symbole != NULL) {
        return symbole;
    }

    symbole = Symbole_new(name);
    HashTable_insert(symtable->table, symbole);

    return symbole;
}

Symbole * newtemp (SymboleTableRoot * root) {
    char name[25];
    int res = snprintf(name, 25, "-T%lu", root->temporary);
    if (res < 0 || res >= 25) {
        perror("newtemp snprintf");
        exit(1);
    }
    (root->temporary)++;
    return newname(root, name);
}



Symbole * Symbole_new(const char * name) {
    Symbole * symbole = malloc(sizeof(Symbole));
    CHECKMALLOC(symbole);
    size_t size = strlen(name) + 1;
    symbole->name = malloc(size);
    CHECKMALLOC(symbole->name);
    strncpy(symbole->name, name, size);
    
    // on différencie selon le type 
    symbole->value.string_lit = NULL;
    symbole->type = T_NONE;
    symbole->kind = K_NONE;
    return symbole;
}

void Symbole_free(Symbole * symbole) {
    free(symbole->name);
    free(symbole);
}


struct Code * Code_new() {
    Code * code = malloc(sizeof(Code));
    CHECKMALLOC(code);
    code->capacity = 1024;
    code->quads = malloc(code->capacity * sizeof(Quad));
    CHECKMALLOC(code->quads);
    code->nextquad = 0;
}

void Code_free(Code * code) {
    free(code->quads);
    free(code);
}


void gencode(Code * code, enum quad_kind k, Symbole * s1, Symbole * s2, Symbole * s3) {
    if (code->nextquad == code->capacity) {
        code->capacity += 1024;
        code->quads = realloc(code->quads, code->capacity * sizeof(Quad));
        if(code->quads == NULL) {
            fprintf(stderr, "Error attempting to grow quad list (actual size is %d)\n",code->nextquad);
            exit(1);
        }
    }

    Quad * q = &(code->quads[code->nextquad]);
    q->kind = k;
    q->sym1 = s1;
    q->sym2 = s2;
    q->sym3 = s3;
    (code->nextquad)++;
}



struct ListSymboles * ListSymboles_new() {
    struct ListSymboles * l = malloc(sizeof(struct ListSymboles));
    CHECKMALLOC(l);
    l->count = 0;
    l->size = 8;
    l->symboles = malloc(l->size * sizeof(Symbole *));
    CHECKMALLOC(l->symboles);
    return l;
}

void ListSymboles_free(struct ListSymboles * l) {
    free(l->symboles);
    free(l);
}

void ListSymboles_add(struct ListSymboles * l, Symbole * symbole) {
    if (l->count == l->size) {
        l->size *= 2;
        l->symboles = realloc(l->symboles, l->size * sizeof(Symbole *));
        CHECKMALLOC(l->symboles);
    }

    l->symboles[l->count] = symbole;
    (l->count)++;
}


FunctionsContexts * FunctionsContexts_new() {
    FunctionsContexts *ctx = malloc(sizeof(FunctionsContexts));
    CHECKMALLOC(ctx);
    ctx->count = 0;
    ctx->size = 8;
    ctx->lists_sym = malloc(ctx->size * sizeof(struct ListSymboles *));
    CHECKMALLOC(ctx->lists_sym);
    return ctx;
}

void FunctionsContexts_free(FunctionsContexts * ctx) {
    for( size_t i = 0; i < ctx->count; i++){
        ListSymboles_free(ctx->lists_sym[i]);
    }
    free(ctx);
}

void FunctionsContexts_push(FunctionsContexts * ctx) {
    if (ctx->count == ctx->size) {
        ctx->size *= 2;
        ctx->lists_sym = realloc(ctx->lists_sym, ctx->size * sizeof(struct ListSymboles *));
        CHECKMALLOC(ctx->lists_sym);
    }

    ctx->lists_sym[ctx->count] = ListSymboles_new();
    (ctx->count)++;
}

void FunctionsContexts_pop(FunctionsContexts * ctx) {
    if (ctx->count > 0) {
        ListSymboles_free(ctx->lists_sym[ctx->count - 1]);
        (ctx->count)--;
    } else {
        fprintf(stderr, "error: FunctionsContexts_pop\n");
        exit(1);
    }
}

void FunctionsContexts_new_var(FunctionsContexts * ctx, Symbole * sym) {
    if (ctx->count > 0) {
        struct ListSymboles * list_sym = ctx->lists_sym[ctx->count-1];
        for (size_t j = 0; j < list_sym->count; j++) {
            Symbole * sym = list_sym->symboles[j];
            sym->offset += 4;
        }
        sym->offset = 0;
        ListSymboles_add(list_sym, sym);
    } else {
        fprintf(stderr, "error: FunctionsContexts_new_var\n");
        exit(1);
    }
}

void genMIPS(FILE * file, Code * code, SymboleTableRoot * symtable, FunctionsContexts * ctx) {
    size_t gv_count = 0;

    fprintf(file, ".text\n");

    for (size_t i = 0; i < code->nextquad; i++) {
        Quad * quad = &(code->quads[i]);
        switch (quad->kind)
        {
        case OP_NFC: // new fonction context
            FunctionsContexts_push(ctx);
            break;

        case OP_DFC: // delete fonction context 
            fprintf(file, "\taddiu $sp, $sp, %lu\n", (ctx->lists_sym[ctx->count - 1])->count * 4);
            fprintf(file, "\tjr $ra\n"); // A CHANGER !! 
            FunctionsContexts_pop(ctx);
            break;

        case OP_GV: // global variable
        {
            quad->sym1->offset = gv_count;
            gv_count++;
        }
            break;

        case OP_LV: // local variable
        {
            FunctionsContexts_new_var(ctx, quad->sym1);
            fprintf(file, "\taddi $sp, $sp, -4\n");
        }
            break;

        case OP_GE:
            break;

        case OP_LE:
            break;

        case OP_NE:
            break;
        
        case OP_GT:
            break;
        
        case OP_LT:
            break;
        
        case OP_ADD:
            fprintf(file, "\tlw $t2, %u($sp)\n", quad->sym2->offset);
            fprintf(file, "\tlw $t3, %u($sp)\n", quad->sym3->offset);
            fprintf(file, "\taddu $t1, $t2, $t3\n");
            fprintf(file, "\tsw $t1, %u($sp)\n", quad->sym1->offset);
            break;
        
        case OP_SUB:
            break;
        
        case OP_MUL:
            break;
        
        case OP_DIV:
            break;
        
        case OP_MOD:
            break;
        
        case OP_INCR:
            break;

        case OP_DECR:
            break;
        
        case OP_EQ:
            break;
        
        case OP_AND:
            break;
        
        case OP_OR:
            break;
        
        case OP_NOT:
            break;
        
        case OP_UMOINS:
            break;
        
        case OP_CALL:
        {
            
        }
            break;
        
        case OP_WS:
            break;
            
        default:
            break;
        }
    }

    genMIPS_data(file, symtable, gv_count);
}

void genMIPS_data(FILE * file, SymboleTableRoot * root, size_t gv_count) {
    fprintf(file, ".data\n");
    fprintf(file, "\t_GV: .space %lu\n", 4 * gv_count);


    SymboleTable * symtable = root->next;
    if (symtable == NULL) {
        fprintf(stderr, "Error : global variable symtable does not exist. Error in the algorithm\n");
        exit(1);
    }
    while (symtable->next != NULL) {
        symtable = symtable->next;
    }

    size_t str_count = 0;
    HashTable *hash = symtable->table;
    for (size_t i = 0; i < hash->size; i++) {
        HashTableBucket * bucket = hash->buckets[i].next;
        while (bucket != NULL) {
            Symbole * sym = bucket->symbole;
            if (sym->kind == K_TAB) {
                fprintf(file, "\t%s: .space %lu\n", sym->name, 4 * sym->value.tab_size);
            } else if (sym->kind == K_CONST && sym->type == T_STRING) {
                fprintf(file, "\t_STR%lu: .asciiz %s\n", str_count, sym->name);
                str_count++;
            }
            bucket = bucket->next;
        }
    }

    
}


// OP_GE, OP_LE, OP_NE, OP_GT, OP_LT,
// OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_MOD,
// OP_INCR, OP_DECR, OP_EQ,
// OP_AND, OP_OR, OP_NOT,
// OP_UMOINS,
// OP_WS,