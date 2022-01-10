%{
#include "decaf.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


extern int yylex();
int yyerror(char*);

struct Symbole;
typedef struct Symbole Symbole;


%}

%union {
    long int intval;
    char * strval;
    struct {
        struct Symbole * ptr;
        struct ListGoto * true;
        struct ListGoto * false;
    } exprval;
    struct {
        unsigned long quad;
    } marker;
    int type;
    struct ListSymboles * list_sym;
}

// %token COMMENT
%token BOOLEAN
%token BREAK
%token CLASS
%token CONTINUE
%token ELSE
%token FOR
%token IF
%token INT
%token RETURN
%token VOID

%token MAIN


// %token PROGRAM
%token GE
%token LE
%token EQ
%token NE
%token INCR
%token DECR
%token AND
%token OR

%token WI
%token RI
%token WB
%token WS

%token <strval> STRING_LIT
%token <intval> CHAR_LIT
%token <intval> BOOL_LIT
%token <intval> INT_LIT

%token <strval> ID

%type <exprval> literal
%type <exprval> expr
%type <exprval> location
%type <exprval> method_call
%type <exprval> var_decl
%type <exprval> field
%type <exprval> method_arg


%type <list_sym> list_id
%type <list_sym> list_field
%type <list_sym> list_method_arg
%type <list_sym> method_arg_opt
%type <list_sym> list_method_call_expr
%type <list_sym> method_call_expr_opt


%type <type> type
%type <type> method_type
%type <type> assign_op


%left OR
%left AND
%left EQ NE
%left '<' '>' LE GE
%left '+' '-'
%left '*' '/' '%'
%left '!'
%left UMOINS

%%

program 
: CLASS program_name '{' field_method '}'
;




program_name
: ID
| WS
| WB
| WI
| RI
;




field_method 
: field_decl field_method

| 
VOID ID 
{
    pushctx(SYMTABLE);
}
'(' method_arg_opt ')'
{
    Symbole * sym = newfunc(SYMTABLE, $2);
    sym->type = T_VOID;
    sym->kind = K_FCT;
    sym->value.args = $5;

    free($2);
    
    gencode(CODE, OP_NFC, sym, NULL, NULL);
}
'{' block_bis '}' 
{
    gencode(CODE, OP_DFC, NULL, NULL, NULL);
    popctx(SYMTABLE);
}
list_method_decl

|
type ID 
{
    pushctx(SYMTABLE);
}
'(' method_arg_opt ')' 
{
    Symbole * sym = newfunc(SYMTABLE, $2);
    if ($1 == INT) {
        sym->type = T_INT;
    } else if ($1 == BOOLEAN){
        sym->type = T_BOOL;
    }
    sym->kind = K_FCT;
    sym->value.args = $5;

    free($2);

    gencode(CODE, OP_NFC, sym, NULL, NULL);
}
'{' block_bis '}' 
{
    gencode(CODE, OP_DFC, NULL, NULL, NULL);
    popctx(SYMTABLE);
}
list_method_decl

| 
VOID MAIN 
{
    pushctx(SYMTABLE);
}
'(' ')' 
{
    Symbole * sym = newfunc(SYMTABLE, "main");
    sym->type = T_VOID;
    sym->kind = K_FCT;
    sym->value.args = ListSymboles_new();

    gencode(CODE, OP_NFC, sym, NULL, NULL);
}
'{' block_bis '}' 
{
    gencode(CODE, OP_DFC, NULL, NULL, NULL);
    popctx(SYMTABLE);
}
;




field_decl 
: type field list_field ';'
{
    ListSymboles_add($3, $2.ptr);

    for (size_t i = 0; i < $3->count; i++) {
        Symbole * sym = ($3->symboles)[i];
        if ($1 == INT) {
            sym->type = T_INT;
        } else if ($1 == BOOLEAN) {
            sym->type = T_BOOL;
        }
        if (sym->kind == K_GLOB) {
            gencode(CODE, OP_GV, sym, NULL, NULL);
        }
    }
    ListSymboles_free($3);  
}
;




list_field 
: ',' field list_field
{
    ListSymboles_add($3, $2.ptr);
    $$ = $3;   
}

|
{
    struct ListSymboles * list_sym = ListSymboles_new();
    $$ = list_sym; 
}
;


  
field 
: ID 
{
    Symbole * sym = newname(SYMTABLE, $1);
    sym->kind = K_GLOB;
    $$.ptr = sym;
}

| ID '[' INT_LIT ']' 
{
    Symbole * sym = newname(SYMTABLE, $1);
    sym->kind = K_TAB;
    sym->value.tab_size = $3;
    $$.ptr = sym;
}
;




list_method_decl 
: method_decl list_method_decl

| 
VOID MAIN 
{
    pushctx(SYMTABLE);
}
'(' ')' 
{
    Symbole * sym = newfunc(SYMTABLE, "main");
    sym->type = T_VOID;
    sym->kind = K_FCT;
    sym->value.args = ListSymboles_new();

    gencode(CODE, OP_NFC, sym, NULL, NULL);
}
'{' block_bis '}' 
{
    gencode(CODE, OP_DFC, NULL, NULL, NULL);
    popctx(SYMTABLE);
}
;




method_decl 
: method_type ID 
{
    pushctx(SYMTABLE);
}
'(' method_arg_opt ')' 
{
    Symbole * sym = newfunc(SYMTABLE, $2);
    if ($1 == INT) {
        sym->type = T_INT;
    } else if ($1 == BOOLEAN){
        sym->type = T_BOOL;
    } else if ($1 == VOID) {
        sym->type = T_VOID;
    }
    sym->kind = K_FCT;
    sym->value.args = $5;

    free($2);

    gencode(CODE, OP_NFC, sym, NULL, NULL);
}
'{' block_bis '}' 
{
    gencode(CODE, OP_DFC, NULL, NULL, NULL);
    popctx(SYMTABLE);
}
;




method_type 
: type 
{
    $$ = $1;
}

| VOID
{
    $$ = VOID;
}
;




method_arg_opt 
: method_arg list_method_arg
{
    ListSymboles_add($2, $1.ptr);
    $$ = $2;
}

|
{
    struct ListSymboles * list_sym = ListSymboles_new();
    $$ = list_sym; 
}
;




list_method_arg 
: ',' method_arg list_method_arg
{
    ListSymboles_add($3, $2.ptr);
    $$ = $3;
}

|
{
    struct ListSymboles * list_sym = ListSymboles_new();
    $$ = list_sym;  
}
;




method_arg 
: type ID
{
    Symbole * sym = newname(SYMTABLE, $2);
    sym->kind = K_VAR;
    if ($1 == INT) {
        sym->type = T_INT;
    } else if ($1 == BOOLEAN) {
        sym->type = T_BOOL;
    }
    free($2);
    $$.ptr = sym;
}
;




block 
: '{'
{
    pushctx(SYMTABLE);
}
block_bis
{
    popctx(SYMTABLE);
}
'}'
;




block_bis
:list_var_decl list_statement
;




 // pas de code car variable gérée dans var_decl
list_var_decl 
: var_decl list_var_decl
|
;




var_decl 
: type ID list_id ';'
{
    Symbole * sym = newname(SYMTABLE, $2);
    sym->kind = K_VAR;
    ListSymboles_add($3, sym);

    for (size_t i = 0; i < $3->count; i++) {
        sym = ($3->symboles)[i];
        if ($1 == INT) {
            sym->type = T_INT;
        } else if ($1 == BOOLEAN) {
            sym->type = T_BOOL;
        }
        gencode(CODE, OP_LV, sym, NULL, NULL);
    }
    ListSymboles_free($3);
}
;




list_id 
: ',' ID list_id
{
    Symbole * sym = newname(SYMTABLE, $2);
    sym->kind = K_VAR;
    ListSymboles_add($3, sym);
    $$ = $3;
}

|
{
    struct ListSymboles * list_sym = ListSymboles_new();
    $$ = list_sym;    
}
;




type 
: INT
{
    $$ = INT;
}

| BOOLEAN
{
    $$ = BOOLEAN;
}
;




list_statement 
: statement list_statement

|
;




statement 
: location assign_op expr ';'
{
    if ($2 == '=') {
        if (!( op_is_int($1.ptr, $3.ptr) || op_is_bool($1.ptr, $3.ptr) )) {
            fprintf(stderr, "type not INT or BOOL for \'=\' \n");
            exit(EXIT_FAILURE);
        }
        gencode(CODE, OP_ASSIGN, $1.ptr, $3.ptr, NULL);

    }  else {
        if (!op_is_int($1.ptr, $3.ptr)) {
            fprintf(stderr, "type not INT for INCR or DECR \n");
            exit(EXIT_FAILURE);
        };
        if ($2 == DECR) {
            gencode(CODE, OP_DECR, $1.ptr, $3.ptr, NULL);
        } else {
            gencode(CODE, OP_INCR, $1.ptr, $3.ptr, NULL);
        }
    }
    
    	
}

| method_call ';'

| IF '(' expr ')' block statement_else_opt
{
    if (($3.ptr)->type != T_BOOL) {
        fprintf(stderr, "if condition type not boolean\n");
        exit(EXIT_FAILURE);
    }

}

| FOR ID '=' expr ',' expr 
{ 
    pushctx(SYMTABLE);

    

    if (!($4.ptr->type == T_INT && $6.ptr->type == T_INT)) {
        fprintf(stderr, "for loop arg not int\n");
        exit(EXIT_FAILURE);
    }

    Symbole * sym = newname(SYMTABLE, $2);
    sym->kind = K_VAR;
    sym->type =T_INT;

    gencode(CODE, OP_ASSIGN, sym, $4.ptr, NULL);

    free($2);
}
block
{
    popctx(SYMTABLE);
}


| RETURN statement_expr_opt ';'

| BREAK ';'

| CONTINUE ';'

| block
;




statement_else_opt 
: ELSE block

|
;




statement_expr_opt 
: expr

|
;
                 



assign_op 
: '='
{
    $$ = '=';
}
| INCR
{
    $$ = INCR;
}

| DECR
{
    $$ = DECR;
}
;




method_call 
: ID '(' method_call_expr_opt ')'
{
    Symbole * sym_fct = lookup(SYMTABLE, $1);
    if (sym_fct == NULL) {
        fprintf(stderr, "ID \"%s\" is not declared\n", $1);
        exit(EXIT_FAILURE);
    }
    if (sym_fct->kind != K_FCT) {
        fprintf(stderr, "ID \"%s\" is not a function\n", $1);
        exit(EXIT_FAILURE);
    }

    ListSymboles * l1 = sym_fct->value.args;
    ListSymboles * l2 = $3;

    if (l1->count != l2->count) {
        fprintf(stderr, "error : arguments number not the same\n");
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < l1->count; i++) {
        if (l1->symboles[i]->type != l2->symboles[i]->type) {
            fprintf(stderr, "error : function call not matching type\n");
            exit(EXIT_FAILURE);
        }
    }

    ListSymboles_free(l2);
    free($1);
    
    // val de retour
    Symbole * sym = newtemp(SYMTABLE);
    sym->type = sym_fct->type;
    sym->kind = K_VAR;
    gencode(CODE, OP_CALL, sym_fct, sym, NULL);
    $$.ptr = sym;
}

| WS '(' STRING_LIT ')'
{
    char name[30];
    int res = snprintf(name, 30, "_STR_%lu", SYMTABLE->temporary);
    (SYMTABLE->temporary)++;
    if (res < 0 || res >= 30) {
        perror("bool_lit snprintf");
        exit(EXIT_FAILURE);
    }
    char * str_id = strndup(name, res);
    if (str_id == NULL) {
        perror("strndup");
        exit(EXIT_FAILURE);
    }
    
    Symbole * sym = newconst(SYMTABLE, $3);
    sym->type = T_STRING;
    sym->kind = K_CONST;
    sym->value.string_lit = str_id;
    free($3);
    gencode(CODE, OP_WS, sym, NULL, NULL);
}

| WI '(' expr ')'
{
    if (($3.ptr)->type != T_INT) {
        fprintf(stderr, "WriteInt argument type is not int\n");
        exit(EXIT_FAILURE);
    }
    gencode(CODE, OP_WI, $3.ptr, NULL, NULL);
}


| WB '(' expr ')'
{
    if (($3.ptr)->type != T_BOOL) {
        fprintf(stderr, "WriteBool argument type is not int\n");
        exit(EXIT_FAILURE);
    }
    gencode(CODE, OP_WB, $3.ptr, NULL, NULL);
}


| RI '(' location ')'
{
    if (($3.ptr)->type != T_INT) {
        fprintf(stderr, "WriteInt argument type is not int\n");
        exit(EXIT_FAILURE);
    }
    gencode(CODE, OP_RI, $3.ptr, NULL, NULL);
}
;




method_call_expr_opt 
: expr list_method_call_expr
{
    ListSymboles_add($2, $1.ptr);
    gencode(CODE, OP_PUSH, $1.ptr, NULL, NULL);
    $$ = $2;
}

|
{
    ListSymboles * list_sym = ListSymboles_new();
    $$ = list_sym;
}
;




list_method_call_expr 
: ',' expr list_method_call_expr
{
    ListSymboles_add($3, $2.ptr);
    gencode(CODE, OP_PUSH, $2.ptr, NULL, NULL);
    $$ = $3;
}

|
{
    ListSymboles * list_sym = ListSymboles_new();
    $$ = list_sym;
}
;




location 
: ID 
{
    Symbole * sym = lookup(SYMTABLE, $1);
    if (sym == NULL) {
        fprintf(stderr, "ID \"%s\" is not declared\n", $1);
        exit(EXIT_FAILURE);
    }

    if (!(sym->kind == K_VAR || sym->kind == K_TAB_IDX || sym->kind == K_GLOB)) {
        fprintf(stderr, "ID %s wrong kind\n", $1);
        exit(EXIT_FAILURE);
    }

    free($1);
    $$.ptr = sym;
}

| ID '[' expr ']'
{
    Symbole * sym = lookup(SYMTABLE, $1);
    if (sym == NULL) {
        fprintf(stderr, "ID \"%s\" is not declared\n", $1);
        exit(EXIT_FAILURE);
    }
    if (sym->kind != K_TAB) {
        fprintf(stderr, "ID \"%s\" is not an array\n", $1);
        exit(EXIT_FAILURE);
    }
    free($1);
    if ($3.ptr->type != T_INT) {
        fprintf(stderr, "not type int to access array \"%s\"\n", $1);
        exit(EXIT_FAILURE);
    }

    Symbole * entry_tab = newtemp(SYMTABLE);
    entry_tab->kind = K_TAB_IDX;
    entry_tab->type = sym->type;
    entry_tab->value.tab[0] = sym;
    entry_tab->value.tab[1] = $3.ptr;
    $$.ptr = entry_tab;
}
;


        

expr 
: location
{
    $$.ptr = $1.ptr;
}

| method_call

| literal 
{
    $$.ptr = $1.ptr;
}

| expr '+' expr
{
    if (!(op_is_int($1.ptr, $3.ptr))) {
        fprintf(stderr, "ADD type not INT\n");
        exit(EXIT_FAILURE);
    };
    Symbole * sym = newtemp(SYMTABLE);
    gencode(CODE, OP_LV, sym, NULL, NULL);
    sym->type = T_INT;
    sym->kind = K_VAR;
    gencode(CODE, OP_ADD, sym, $1.ptr, $3.ptr);
    $$.ptr = sym;
}

| expr '-' expr 
{
    if (!(op_is_int($1.ptr, $3.ptr))) {
        fprintf(stderr, "SUB type not INT\n");
        exit(EXIT_FAILURE);
    };
    Symbole * sym = newtemp(SYMTABLE);
    gencode(CODE, OP_LV, sym, NULL, NULL);
    sym->type = T_INT;
    sym->kind = K_VAR;
    gencode(CODE, OP_SUB, sym, $1.ptr, $3.ptr);
    $$.ptr = sym;
}

| expr '*' expr
{
    if (!(op_is_int($1.ptr, $3.ptr))) {
        fprintf(stderr, "MUL type not INT\n");
        exit(EXIT_FAILURE);
    };
    Symbole * sym = newtemp(SYMTABLE);
    gencode(CODE, OP_LV, sym, NULL, NULL);
    sym->type = T_INT;
    sym->kind = K_VAR;
    gencode(CODE, OP_MUL, sym, $1.ptr, $3.ptr);
    $$.ptr = sym;
}

| expr '/' expr
{
    if (!(op_is_int($1.ptr, $3.ptr))) {
        fprintf(stderr, "DIV type not INT\n");
        exit(EXIT_FAILURE);
    };
    Symbole * sym = newtemp(SYMTABLE);
    gencode(CODE, OP_LV, sym, NULL, NULL);
    sym->type = T_INT;
    sym->kind = K_VAR;
    gencode(CODE, OP_DIV, sym, $1.ptr, $3.ptr);
    $$.ptr = sym;
}

| expr '%' expr
{
    if (!(op_is_int($1.ptr, $3.ptr))) {
        fprintf(stderr, "MOD type not INT\n");
        exit(EXIT_FAILURE);
    };
    Symbole * sym = newtemp(SYMTABLE);
    gencode(CODE, OP_LV, sym, NULL, NULL);
    sym->type = T_INT;
    sym->kind = K_VAR;
    gencode(CODE, OP_MOD, sym, $1.ptr, $3.ptr);
    $$.ptr = sym;
}

| expr '<' expr
{
    if (!(op_is_int($1.ptr, $3.ptr))) {
        fprintf(stderr, "LT type not INT\n");
        exit(EXIT_FAILURE);
    };
    Symbole * sym = newtemp(SYMTABLE);
    gencode(CODE, OP_LV, sym, NULL, NULL);
    sym->type = T_BOOL;
    sym->kind = K_VAR;
    gencode(CODE, OP_LT, sym, $1.ptr, $3.ptr);
    $$.ptr = sym;
}

| expr '>' expr
{
    if (!(op_is_int($1.ptr, $3.ptr))) {
        fprintf(stderr, "GT type not INT\n");
        exit(EXIT_FAILURE);
    };
    Symbole * sym = newtemp(SYMTABLE);
    gencode(CODE, OP_LV, sym, NULL, NULL);
    sym->type = T_BOOL;
    sym->kind = K_VAR;
    gencode(CODE, OP_GT, sym, $1.ptr, $3.ptr);
    $$.ptr = sym;
}

| expr LE expr
{
    if (!(op_is_int($1.ptr, $3.ptr))) {
        fprintf(stderr, "LE type not INT\n");
        exit(EXIT_FAILURE);
    };
    Symbole * sym = newtemp(SYMTABLE);
    gencode(CODE, OP_LV, sym, NULL, NULL);
    sym->type = T_BOOL;
    sym->kind = K_VAR;
    gencode(CODE, OP_LE, sym, $1.ptr, $3.ptr);
    $$.ptr = sym;
}

| expr GE expr
{
    if (!(op_is_int($1.ptr, $3.ptr))) {
        fprintf(stderr, "GE type not INT\n");
        exit(EXIT_FAILURE);
    };
    Symbole * sym = newtemp(SYMTABLE);
    gencode(CODE, OP_LV, sym, NULL, NULL);
    sym->type = T_BOOL;
    sym->kind = K_VAR;
    gencode(CODE, OP_GE, sym, $1.ptr, $3.ptr);
    $$.ptr = sym;
}

| expr EQ expr
{
    if (!( op_is_int($1.ptr, $3.ptr) || op_is_bool($1.ptr, $3.ptr) )) {
        fprintf(stderr, "EQ type not INT or BOOL\n");
        exit(EXIT_FAILURE);
    };
    Symbole * sym = newtemp(SYMTABLE);
    gencode(CODE, OP_LV, sym, NULL, NULL);
    sym->type = T_BOOL;
    sym->kind = K_VAR;
    gencode(CODE, OP_EQ, sym, $1.ptr, $3.ptr);
    $$.ptr = sym;
}

| expr NE expr
{
    if (!( op_is_int($1.ptr, $3.ptr) || op_is_bool($1.ptr, $3.ptr) )) {
        fprintf(stderr, "NE type not INT or BOOL\n");
        exit(EXIT_FAILURE);
    };
    Symbole * sym = newtemp(SYMTABLE);
    gencode(CODE, OP_LV, sym, NULL, NULL);
    sym->type = T_BOOL;
    sym->kind = K_VAR;
    gencode(CODE, OP_NE, sym, $1.ptr, $3.ptr);
    $$.ptr = sym;
}

| expr AND expr
{
    if (!(op_is_bool($1.ptr, $3.ptr))) {
        fprintf(stderr, "AND type not BOOL\n");
        exit(EXIT_FAILURE);
    };
    Symbole * sym = newtemp(SYMTABLE);
    gencode(CODE, OP_LV, sym, NULL, NULL);
    sym->type = T_BOOL;
    sym->kind = K_VAR;
    gencode(CODE, OP_AND, sym, $1.ptr, $3.ptr);
    $$.ptr = sym;
}

| expr OR expr
{
    if (!(op_is_bool($1.ptr, $3.ptr))) {
        fprintf(stderr, "OR type not BOOL\n");
        exit(EXIT_FAILURE);
    };
    Symbole * sym = newtemp(SYMTABLE);
    gencode(CODE, OP_LV, sym, NULL, NULL);
    sym->type = T_BOOL;
    sym->kind = K_VAR;
    gencode(CODE, OP_OR, sym, $1.ptr, $3.ptr);
    $$.ptr = sym;
}

| '!' expr
{
    if (!( ($2.ptr)->type == T_BOOL )) {
        fprintf(stderr, "NOT type not BOOL\n");
        exit(EXIT_FAILURE);
    };
    Symbole * sym = newtemp(SYMTABLE);
    gencode(CODE, OP_LV, sym, NULL, NULL);
    sym->type = T_BOOL;
    sym->kind = K_VAR;
    gencode(CODE, OP_NOT, sym, $2.ptr, NULL);
    $$.ptr = sym;
}

| '-' expr 
{
	if (!( ($2.ptr)->type == T_INT )) {
        fprintf(stderr, "UMOINS type not INT\n");
        exit(EXIT_FAILURE);
    };
    Symbole * sym = newtemp(SYMTABLE);
    gencode(CODE, OP_LV, sym, NULL, NULL);
    sym->type = T_INT;
    sym->kind = K_VAR;
    gencode(CODE, OP_UMOINS, sym, $2.ptr, NULL);
} %prec UMOINS

| '(' expr ')'
{
    $$.ptr = $2.ptr;
}
;  




literal 
: INT_LIT 
{
    char name[20];
    int res = snprintf(name, 20, "-I%li", $1);
    if (res < 0 || res >= 20) {
        perror("int_lit snprintf");
        exit(EXIT_FAILURE);
    }
    Symbole * sym = newconst(SYMTABLE, name);
    sym->type = T_INT;
    sym->kind = K_CONST;
    sym->value.int_lit = $1;
    $$.ptr = sym;
}

| CHAR_LIT 
{
    char name[20];
    int res = snprintf(name, 20, "-C%li", $1);
    if (res < 0 || res >= 20) {
        perror("char_lit snprintf");
        exit(EXIT_FAILURE);
    }
    Symbole * sym = newconst(SYMTABLE, name);
    sym->type = T_CHAR;
    sym->kind = K_CONST;
    sym->value.char_lit = $1;
    $$.ptr = sym;
}

| BOOL_LIT 
{
    char name[20];
    int res = snprintf(name, 20, "-B%li", $1);
    if (res < 0 || res >= 20) {
        perror("bool_lit snprintf");
        exit(EXIT_FAILURE);
    }
    Symbole * sym = newconst(SYMTABLE, name);
    sym->type = T_BOOL;
    sym->kind = K_CONST;
    sym->value.bool_lit = $1;
    $$.ptr = sym;
}
;

%%



int yyerror(char* s) {
    fprintf(stderr, "error : %s\n", s);
    return 1;
}
