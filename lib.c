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

#define CHECKMALLOC(op) if (op == NULL) { perror(#op); exit(EXIT_FAILURE); }
#define INTI_SIZE 64LU;

int op_is_int(Symbole * sym1, Symbole * sym2) {
    return (sym1->type == T_INT && sym2->type == T_INT);
}

int op_is_bool(Symbole * sym1, Symbole * sym2) {
    return (sym1->type == T_BOOL && sym2->type == T_BOOL);
}


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
    hashtable->size = INTI_SIZE;
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
   return h;
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
    root->poped = NULL;

    return root;
}

void SymboleTableRoot_free(SymboleTableRoot * root) {
    SymboleTable * symtable = root->next;
    while (symtable != NULL) {
        symtable = SymboleTable_free(symtable);
    }

    symtable = root->poped;
    while (symtable != NULL) {
        symtable = SymboleTable_free(symtable);
    }
    
    free(root);
}

void SymboleTableRoot_dump(SymboleTableRoot * root) {
    SymboleTable * symtable = root->next;
    // ajout de poped
    SymboleTable * poped = root->poped;
    while (symtable != NULL) {
        HashTable_dump(symtable->table);
        HashTable_dump(poped->table);
        printf("\n");
        symtable = symtable->next;
        poped = poped->next;
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

    symtable->next = root->poped;
    root->poped = symtable;
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
        exit(EXIT_FAILURE);
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
        exit(EXIT_FAILURE);
    }
    (root->temporary)++;
    return newname(root, name);
}

Symbole * newfunc(SymboleTableRoot * root, const char * name) {
    SymboleTable * symtable = root->next;
    while (symtable->next != NULL) {
        symtable = symtable->next;
    }

    Symbole * symbole = HashTable_get(symtable->table, name);
    if (symbole != NULL) {
        fprintf(stderr, "error: function %s aleardy declared\n", name);
        exit(EXIT_FAILURE);        
    }

    symbole = Symbole_new(name);
    HashTable_insert(symtable->table, symbole);

    return symbole;
}

Symbole * newquadsym(SymboleTableRoot * root) {
    Symbole * sym = newtemp(root);
    sym->kind = K_QUAD;
    sym->value.idx_quad = SIZE_MAX;
    return sym;
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
    if (symbole->kind == K_FCT) {
        ListSymboles_free(symbole->value.args);
    } else if (symbole->kind == K_CONST && symbole->type == T_STRING) {
        free(symbole->value.string_lit);
    }
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
        CHECKMALLOC(code->quads);
    }

    Quad * q = &(code->quads[code->nextquad]);
    q->kind = k;
    q->sym1 = s1;
    q->sym2 = s2;
    q->sym3 = s3;
    q->label = 0;
    (code->nextquad)++;
}



ListGoto * ListGoto_new() {
    ListGoto * lg = malloc(sizeof(ListGoto));
    CHECKMALLOC(lg);
    lg->count = 0;
    lg->size = INTI_SIZE;
    lg->quads_idx = malloc(lg->size * sizeof(size_t));
    CHECKMALLOC(lg->quads_idx);
    return lg;
}

void ListGoto_free(ListGoto * lg) {
    free(lg->quads_idx);
    free(lg);
}

void ListGoto_add(ListGoto * lg, size_t quad_idx) {
    if (lg->count == lg->size) {
        lg->size *= 2;
        lg->quads_idx = realloc(lg->quads_idx, lg->size * sizeof(size_t));
        CHECKMALLOC(lg->quads_idx);
    }

    lg->quads_idx[lg->count] = quad_idx;
    (lg->count)++;
}

ListGoto * ListGoto_concat(ListGoto * lg1,  ListGoto * lg2) {
    if (lg1->count == 0) {
        ListGoto_free(lg1);
        return lg2;
    }

    if (lg2->count == 0) {
        ListGoto_free(lg2);
        return lg1;
    }


    ListGoto * lg = malloc(sizeof(ListGoto));
    lg->count = lg1->count + lg2->count;
    lg->size = INTI_SIZE;
    while (lg->size < lg->count) {
        lg->size *= 2;
    }
    lg->quads_idx = malloc(lg->size * sizeof(size_t));
    CHECKMALLOC(lg->quads_idx);

    memcpy(lg->quads_idx, lg1->quads_idx, lg1->count);
    memcpy(lg->quads_idx + lg1->count, lg2->quads_idx, lg2->count);
    
    ListGoto_free(lg1);
    ListGoto_free(lg2);

    return lg;
}

void ListGoto_complete(Code * code, ListGoto * lg, size_t quad_idx) {
    for (size_t i = 0; i < lg->count; i++) {
        Quad * q = &(code->quads[lg->quads_idx[i]]);
        if (!( q->kind == OP_GOTO || q->kind == OP_GOTO_IF || q->kind == OP_GOTO_FOR )) {
            fprintf(stderr, "error compilator branching\n");
            exit(EXIT_FAILURE);
        } 
        if (q->sym3->offset == SIZE_MAX) {
            q->sym3->offset = quad_idx;
        }
    }
    ListGoto_free(lg);
}



struct ListSymboles * ListSymboles_new() {
    struct ListSymboles * l = malloc(sizeof(struct ListSymboles));
    CHECKMALLOC(l);
    l->count = 0;
    l->size = INTI_SIZE;
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
        l->symboles= realloc(l->symboles, l->size * sizeof(Symbole *));
        CHECKMALLOC(l->symboles);
    }

    l->symboles[l->count] = symbole;
    (l->count)++;
}

FunctionsContexts * FunctionsContexts_new() {
    FunctionsContexts *ctx = malloc(sizeof(FunctionsContexts));
    CHECKMALLOC(ctx);
    ctx->count = 0;
    ctx->size = INTI_SIZE;
    ctx->list_ctx = malloc(ctx->size * sizeof(Context));
    CHECKMALLOC(ctx->list_ctx);
    return ctx;
}

void FunctionsContexts_free(FunctionsContexts * ctx) {
    for( size_t i = 0; i < ctx->count; i++){
        ListSymboles_free(ctx->list_ctx[i].list_sym);
    }
    free(ctx->list_ctx);
    free(ctx);
}

void FunctionsContexts_push(FunctionsContexts * ctx) {
    if (ctx->count == ctx->size) {
        ctx->size *= 2;
        ctx->list_ctx = realloc(ctx->list_ctx, ctx->size * sizeof(Context));
        CHECKMALLOC(ctx->list_ctx);
    }

    ctx->list_ctx[ctx->count].list_sym = ListSymboles_new();
    ctx->list_ctx[ctx->count].ret = NULL;
    (ctx->count)++;
}

void FunctionsContexts_pop(FunctionsContexts * ctx) {
    if (ctx->count > 0) {
        ListSymboles_free(ctx->list_ctx[ctx->count - 1].list_sym);
        (ctx->count)--;
    } else {
        fprintf(stderr, "error: FunctionsContexts_pop\n");
        exit(EXIT_FAILURE);
    }
}

void FunctionsContexts_new_var(FunctionsContexts * ctx, Symbole * sym) {
    if (ctx->count > 0) {
        struct ListSymboles * list_sym = ctx->list_ctx[ctx->count-1].list_sym;
        for (size_t j = 0; j < list_sym->count; j++) {
            Symbole * sym = list_sym->symboles[j];
            sym->offset += 4;
        }
        sym->offset = 0;
        ListSymboles_add(list_sym, sym);
    } else {
        fprintf(stderr, "error: FunctionsContexts_new_var\n");
        exit(EXIT_FAILURE);
    }
}

void genMIPS(FILE * file, Code * code, SymboleTableRoot * symtable, FunctionsContexts * ctx) {
    // TODO : uncomment when branch working
    // for (size_t i = 0; i < code->nextquad; i++) {
    //     Quad * quad = &(code->quads[i]);
    //     if (quad->kind == OP_GOTO || quad->kind == OP_GOTO_IF || quad->kind == OP_GOTO_FOR) {
    //         if (quad->sym3->value.idx_quad != SIZE_MAX && quad->sym3->value.idx_quad < code->nextquad) {
    //             code->quads[quad->sym3->offset].label = 1;
    //         } 
    //         else {
    //             fprintf(stderr, "remaining undefined goto\n");
    //             exit(EXIT_FAILURE);
    //         }
    //     }
    // }


    size_t gv_offset = 0;

    fprintf(file, ".text\n");
    fprintf(file, "\tlw $ra _exit\n");
    fprintf(file, "\tb main\n");

    for (size_t idx_quad = 0; idx_quad < code->nextquad; idx_quad++) {
        Quad * quad = &(code->quads[idx_quad]);
        // TODO : uncomment when branch wroking        
        // if (quad->label) {
        //     fprintf(file, "_QUAD%lu:\n", idx_quad);
        // }
        switch (quad->kind)
        {
        case OP_NFC: // new fonction context
        {
            FunctionsContexts_push(ctx);
            Symbole * ret = newtemp(symtable);
            ListSymboles * args = quad->sym1->value.args;
            for (size_t i = 0; i < args->count; i++) {
                FunctionsContexts_new_var(ctx, args->symboles[i]);
            }

            // function label
            fprintf(file, "%s:\n", quad->sym1->name);

            // store ret addr
            FunctionsContexts_new_var(ctx, ret);
            ctx->list_ctx[ctx->count-1].ret = ret;
            fprintf(file, "\taddi $sp, $sp, -4\n");
            fprintf(file, "\tsw $ra, ($sp)\n");
        }
            break;

        case OP_DFC: // delete fonction context 
            fprintf(file, "\tlw $ra, %lu($sp)\n", ctx->list_ctx[ctx->count - 1].ret->offset);
            fprintf(file, "\taddiu $sp, $sp, %lu\n", (ctx->list_ctx[ctx->count - 1].list_sym)->count * 4);
            fprintf(file, "\tjr $ra\n");
            FunctionsContexts_pop(ctx);
            break;

        case OP_GV: // global variable
        {
            quad->sym1->offset = gv_offset;
            gv_offset += 4;
        }
            break;

        case OP_LV: // local variable
        {
            FunctionsContexts_new_var(ctx, quad->sym1);
            fprintf(file, "\taddi $sp, $sp, -4\n");
        }
            break;

        case OP_GE:
            genMIPS_inst_load(file, "$t2", quad->sym2);
            genMIPS_inst_load(file, "$t3", quad->sym3);
            fprintf(file,"\tsge $t1, $t2, $t3\n");
            genMIPS_inst_store(file, "$t1", quad->sym1);
            break;

        case OP_LE:
            genMIPS_inst_load(file, "$t2", quad->sym2);
            genMIPS_inst_load(file, "$t3", quad->sym3);
            fprintf(file,"\tsle $t1, $t2, $t3\n");
            genMIPS_inst_store(file, "$t1", quad->sym1);
            
            break;

        case OP_NE:
            genMIPS_inst_load(file, "$t2", quad->sym2);
            genMIPS_inst_load(file, "$t3", quad->sym3);
            fprintf(file,"\tsne $t1, $t2, $t3\n");
            genMIPS_inst_store(file, "$t1", quad->sym1);
            break;
        
        case OP_GT:
            genMIPS_inst_load(file, "$t2", quad->sym2);
            genMIPS_inst_load(file, "$t3", quad->sym3);
            fprintf(file,"\tsgt $t1, $t2, $t3\n");
            genMIPS_inst_store(file, "$t1", quad->sym1);
            break;
        
        case OP_LT:
            genMIPS_inst_load(file, "$t2", quad->sym2);
            genMIPS_inst_load(file, "$t3", quad->sym3);
            fprintf(file,"\tslt $t1, $t2, $t3\n");
            genMIPS_inst_store(file, "$t1", quad->sym1);
            break;

        case OP_EQ:
            genMIPS_inst_load(file, "$t2", quad->sym2);
            genMIPS_inst_load(file, "$t3", quad->sym3);
            fprintf(file,"\tseq $t1, $t2, $t3\n");
            genMIPS_inst_store(file, "$t1", quad->sym1);
            break;
        
        case OP_ADD:
            genMIPS_inst_load(file, "$t2", quad->sym2);
            genMIPS_inst_load(file, "$t3", quad->sym3);
            fprintf(file, "\tadd $t1, $t2, $t3\n");
            genMIPS_inst_store(file, "$t1", quad->sym1);
            break;
        
        case OP_SUB:
            genMIPS_inst_load(file, "$t2", quad->sym2);
            genMIPS_inst_load(file, "$t3", quad->sym3);
            fprintf(file, "\tsub $t1, $t2, $t3\n");
            genMIPS_inst_store(file, "$t1", quad->sym1);

            break;
        
        case OP_MUL:
            genMIPS_inst_load(file, "$t2", quad->sym2);
            genMIPS_inst_load(file, "$t3", quad->sym3);
            fprintf(file, "\tmul $t1, $t2, $t3\n");
            genMIPS_inst_store(file, "$t1", quad->sym1);
            break;
        
        case OP_DIV:
            genMIPS_inst_load(file, "$a2", quad->sym3); // doit être != 0
            fprintf(file, "\tjal _DIV_ZERO\n");
            genMIPS_inst_load(file, "$t2", quad->sym2);
            fprintf(file, "\tdiv $t1, $t2, $a2\n");
            genMIPS_inst_store(file, "$t1", quad->sym1);
            
            break;
        
        case OP_MOD:
            genMIPS_inst_load(file, "$a2", quad->sym3);
            fprintf(file, "\tjal _DIV_ZERO\n");
            genMIPS_inst_load(file, "$t2", quad->sym2);
            fprintf(file, "\trem $t1, $t2, $a2\n");
            genMIPS_inst_store(file, "$t1", quad->sym1);
            break;
        
        case OP_INCR:
            genMIPS_inst_load(file, "$t2", quad->sym1);
            genMIPS_inst_load(file, "$t3", quad->sym2); 
            fprintf(file, "\tadd $t1, $t2, $t3\n");
            genMIPS_inst_store(file, "$t1", quad->sym1);
            break;

        case OP_DECR:
            genMIPS_inst_load(file, "$t2", quad->sym1);
            genMIPS_inst_load(file, "$t3", quad->sym2);
            fprintf(file, "\tsub $t1, $t2, $t3\n");
            genMIPS_inst_store(file, "$t1", quad->sym1);
            break;

        case OP_ASSIGN:
            genMIPS_inst_load(file, "$t1", quad->sym2);
            genMIPS_inst_store(file, "$t1", quad->sym1);
            break;
        
        case OP_AND:
            genMIPS_inst_load(file, "$t2", quad->sym2);
            genMIPS_inst_load(file, "$t3", quad->sym3);
            fprintf(file, "\tand $t1, $t2, $t3\n");
            genMIPS_inst_store(file, "$t1", quad->sym1);
            break;

        case OP_OR:
            genMIPS_inst_load(file, "$t2", quad->sym2);
            genMIPS_inst_load(file, "$t3", quad->sym3);
            fprintf(file, "\tor $t1, $t2, $t3\n");
            genMIPS_inst_store(file, "$t1", quad->sym1);
            break;
        
        case OP_NOT:
            genMIPS_inst_load(file, "$t1", quad->sym2);
            fprintf(file, "\tnot $t2, $t1\n");
            genMIPS_inst_store(file, "$t2", quad->sym1);
            break;
        
        case OP_UMOINS:
            genMIPS_inst_load(file, "$t2", quad->sym2);
            fprintf(file, "\tnegu $t1, $t2\n");
            genMIPS_inst_store(file, "$t1", quad->sym1);
            break;
        
        case OP_CALL:
            fprintf(file, "\tjal %s\n", quad->sym1->name);
            genMIPS_inst_store(file, "$v1", quad->sym2);
            break;
        case OP_WS:
            fprintf(file, "\tla $a3, %s\n", quad->sym1->value.string_lit);
            fprintf(file, "\tjal _WriteString\n");
            break;
        case OP_WI:
            genMIPS_inst_load(file, "$a3", quad->sym1);
            fprintf(file, "\tjal _WriteInt\n");
            break;
        case OP_WB:
            genMIPS_inst_load(file, "$a3", quad->sym1);
            fprintf(file, "\tjal _WriteBool\n");
            break;
        case OP_RI:
            fprintf(file, "\tjal _ReadInt\n");
            genMIPS_inst_store(file, "$v0", quad->sym1);
            break;
        case OP_PUSH:
            genMIPS_inst_load(file, "$t1", quad->sym1);
            fprintf(file, "\taddi $sp, $sp, -4\n");
            fprintf(file, "\tsw $t1 ($sp)\n");
            break;
        // to uncomment when branch working
        // case OP_GOTO:
        //     fprintf(file, "\tb _QUAD%lu\n", quad->sym3->value.idx_quad);
        //     break;
        // case OP_GOTO_IF:
        //     genMIPS_inst_load(file, "$t4", quad->sym1);
        //     fprintf(file, "\tbnez $t4  _QUAD%lu\n", quad->sym3->value.idx_quad);
        //     break;
        // case OP_GOTO_FOR:
        //     genMIPS_inst_load(file, "$t4", quad->sym1);
        //     genMIPS_inst_load(file, "$t5", quad->sym2);
        //     fprintf(file, "\tble $4, $5, _QUAD%lu", quad->sym3->value.idx_quad);
        //     break;
        default:
            break;
        }
    }

    // Check tab index
    fprintf(file, "_TAB_CHECK_IDX:\n");
    fprintf(file, "\tbltz $a2, _TAB_ERR\n");
    fprintf(file, "\tbge $a2, $a3 _TAB_ERR\n");
    fprintf(file, "\tjr $ra\n");
    fprintf(file, "_TAB_ERR:\n");
    fprintf(file, "\tli $v0, 4\n");
    fprintf(file, "\tla $a0, _SYS_MSG1\n");
    fprintf(file, "\tsyscall\n");
    fprintf(file, "\tb _exit\n");


    // Check div by zero
    fprintf(file, "_DIV_ZERO:");
    fprintf(file, "\tbeqz $a2, _TAB_ERR\n");
    fprintf(file, "\tjr $ra\n");
    fprintf(file, "_DIV_ZERO_ERR:\n");
    fprintf(file, "\tli $v0, 4\n");
    fprintf(file, "\tla $a0, _SYS_MSG2\n");
    fprintf(file, "\tsyscall\n");
    fprintf(file, "\tb _exit\n"); 


    // WriteString
    fprintf(file, "_WriteString:\n");
    fprintf(file, "\tli $v0, 4\n");
    fprintf(file, "\tmove $a0, $a3\n");
    fprintf(file, "\tsyscall\n");
    fprintf(file, "\tjr $ra\n");


    // WriteInt
    fprintf(file, "_WriteInt:\n");
    fprintf(file, "\tli $v0, 1\n");
    fprintf(file, "\tmove $a0, $a3\n");
    fprintf(file, "\tsyscall\n");
    fprintf(file, "\tjr $ra\n");


    // WriteBool
    fprintf(file, "_WriteBool:\n");
    fprintf(file, "\tli $v0, 4\n");

    fprintf(file, "\tbeqz $a3 _WriteBool_if_zero\n");
    fprintf(file, "\tla $a0, _true\n");
    fprintf(file, "\tb _WriteBool_endif\n");
    fprintf(file, "_WriteBool_if_zero:\n");
    fprintf(file, "\tla $a0, _false\n");
    fprintf(file, "_WriteBool_endif:\n");

    fprintf(file, "\tsyscall\n");
    fprintf(file, "\tjr $ra\n");


    // ReadInt
    fprintf(file, "_ReadInt:\n");
    fprintf(file, "\tli $v0, 5\n");
    fprintf(file, "\tsyscall\n");
    fprintf(file, "\tjr $ra\n");

    // exit code
    fprintf(file, "_exit:\n");
    fprintf(file, "\tli $v0 10\n");
    fprintf(file, "\tsyscall\n");
    genMIPS_data(file, symtable, gv_offset);
}

void genMIPS_data(FILE * file, SymboleTableRoot * root, size_t gv_offset) {
    fprintf(file, ".data\n");

    // global variable
    fprintf(file, "\t_GV: .space %lu\n", gv_offset);

    SymboleTable * symtable = root->next;
    if (symtable == NULL) {
        fprintf(stderr, "Error : global variable symtable does not exist. Error in the algorithm\n");
        exit(EXIT_FAILURE);
    }
    while (symtable->next != NULL) {
        symtable = symtable->next;
    }

    // array
    HashTable *hash = symtable->table;
    for (size_t i = 0; i < hash->size; i++) {
        HashTableBucket * bucket = hash->buckets[i].next;
        while (bucket != NULL) {
            Symbole * sym = bucket->symbole;
            if (sym->kind == K_TAB) {
                fprintf(file, "\t%s: .space %lu\n", sym->name, 4 * sym->value.tab_size);
            }
            bucket = bucket->next;
        }
    }

    // str : add last for memory alignment
    hash = symtable->table;
    for (size_t i = 0; i < hash->size; i++) {
        HashTableBucket * bucket = hash->buckets[i].next;
        while (bucket != NULL) {
            Symbole * sym = bucket->symbole;
            if (sym->kind == K_CONST && sym->type == T_STRING) {
                fprintf(file, "\t%s: .asciiz %s\n", sym->value.string_lit, sym->name);
            }
            bucket = bucket->next;
        }
    }

    fprintf(file, "\t_SYS_MSG1: .asciiz \"Index out of range\"\n");
    fprintf(file, "\t_SYS_MSG2: .asciiz \"Division by zero\"\n");
    fprintf(file, "\t_true: .asciiz \"true\"\n");
    fprintf(file, "\t_false: .asciiz \"false\"\n");
    
}

void genMIPS_inst_load(FILE * file, const char * reg, Symbole * sym) {
    switch (sym->kind) {
    case K_VAR:
        fprintf(file, "\tlw %s, %lu($sp)\n", reg, sym->offset);
        break;
    case K_GLOB:
        fprintf(file, "\tlw %s, _GV+%lu\n", reg, sym->offset);
        break;
    case K_CONST:
        switch (sym->type) {
        case T_INT:
            fprintf(file, "\tli %s, %d\n", reg, sym->value.int_lit);
            break;
        case T_BOOL:
            fprintf(file, "\tli %s, %d\n", reg, sym->value.bool_lit);
            break;
        case T_CHAR:
            fprintf(file, "\tli %s, %d\n", reg, sym->value.char_lit);
            break;
        default:
            fprintf(stderr, "error const type");
            exit(EXIT_FAILURE);
            break;
        }
        break;
    case K_TAB_IDX:
        // get and check index value
        genMIPS_inst_load(file, "$a2", sym->value.tab[1]);
        fprintf(file, "\tli $a3, %lu\n", sym->value.tab[0]->value.tab_size);
        fprintf(file, "\tjal _TAB_CHECK_IDX\n");
        fprintf(file, "\tmulou $t9, $a2, 4\n");
        fprintf(file, "\tlw %s, %s+0($t9)\n", reg, sym->value.tab[0]->name);
        break;
    default:
        fprintf(stderr, "error genMIPS_inst_load : symbole kind not handled\n");
        exit(EXIT_FAILURE);
        break;
    }
}


void genMIPS_inst_store(FILE * file, const char * reg, Symbole * sym) {
    switch (sym->kind) {
    case K_VAR:
        fprintf(file, "\tsw %s, %lu($sp)\n", reg, sym->offset);
        break;
    case K_GLOB:
        fprintf(file, "\tsw %s, _GV+%lu\n", reg, sym->offset);
        break;
    case K_TAB_IDX:
        // get and check index value
        genMIPS_inst_load(file, "$a2", sym->value.tab[1]);
        fprintf(file, "\tli $a3, %lu\n", sym->value.tab[0]->value.tab_size);
        fprintf(file, "\tjal _TAB_CHECK_IDX\n");
        fprintf(file, "\tmulou $t9, $a2, 4\n");
        fprintf(file, "\tsw %s, %s+0($t9)\n", reg, sym->value.tab[0]->name);
        break;
    default:
        fprintf(stderr, "error genMIPS_print_inst_store : symbole kind \"%d\" not handled\n", sym->kind);
        exit(EXIT_FAILURE);
        break;
    }
}

// void genMIPS_goto(FILE * file, ...)

// OP_GE, OP_LE, OP_NE, OP_GT, OP_LT,
// OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_MOD,
// OP_INCR, OP_DECR, OP_EQ,
// OP_AND, OP_OR, OP_NOT,
// OP_UMOINS,
// OP_WS,