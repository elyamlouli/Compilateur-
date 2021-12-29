%{
#include "decaf.h"
#include <stdio.h>

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
    } exprval;
    int type;
    struct ListNames * list_names;
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
%type <exprval> method_name
%type <exprval> var_decl

%type <list_names> list_id


%type <type> type
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
: CLASS ID '{' field_method '}'
;




field_method 
: field_decl field_method

| VOID ID '(' method_arg_opt ')' block list_method_decl

| type ID '(' method_arg_opt ')' block list_method_decl

| VOID MAIN '(' ')' block
;




field_decl 
: type field list_field ';'
;




list_field 
: ',' field list_field
           
|
;


  
field 
: ID 

| ID '[' INT_LIT ']'
;




list_method_decl 
: method_decl list_method_decl

| VOID MAIN '(' ')' block 
;




method_decl 
: method_type ID '(' method_arg_opt ')' block
;




method_type 
: type 

| VOID
;




method_arg_opt 
: method_arg list_method_arg

|
;




list_method_arg 
: ',' method_arg list_method_arg

|
;




method_arg 
: type ID
;




block 
: '{' list_var_decl list_statement '}'
;




list_var_decl 
: var_decl list_var_decl

|
;




var_decl 
: type ID list_id ';'
{
    ListNames_add($3, $2);

    for (size_t i = 0; i < $3->count; i++) {
        char * name = $3->names[i];
        Symbole * sym = newname(SYMTABLE, name);
        if ($1 == INT) {
            sym->type = VAR_INT;
        } else if ($1 == BOOLEAN) {
            sym->type = VAR_BOOL;
        }
    }
    ListNames_free($3);
}
;




list_id 
: ',' ID list_id
{
    ListNames_add($3, $2);
    $$ = $3;
}

|
{
    struct ListNames * list_names = ListNames_new();
    $$ = list_names;    
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
        if (!(
               ((($1.ptr)->type == CONST_INT || ($1.ptr)->type == VAR_INT) && (($3.ptr)->type == CONST_INT || ($3.ptr)->type == VAR_INT)) 
            || ((($1.ptr)->type == CONST_BOOL || ($1.ptr)->type == VAR_BOOL) && (($3.ptr)->type == CONST_BOOL || ($3.ptr)->type == VAR_BOOL)) 
        )) {
            fprintf(stderr, "type not INT or BOOL for \'=\' \n");
            exit(1);
        }
        gencode(CODE, OP_EQ, $1.ptr, $3.ptr, NULL);

    }  else {
        if (!(  (($1.ptr)->type == CONST_INT || ($1.ptr)->type == VAR_INT)
            && (($3.ptr)->type == CONST_INT || ($3.ptr)->type == VAR_INT))) {
            fprintf(stderr, "type not INT for INCR or DECR \n");
            exit(1);
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

| FOR ID '=' expr ',' expr block

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
: method_name '(' method_call_expr_opt ')'

| WS '(' STRING_LIT ')'
{
    Symbole * sym_str = newconst(SYMTABLE, $3);
    sym_str->type = CONST_STRING;
    sym_str->value.string_lit = sym_str->name;
    free($3);
}
;




method_name 
: ID
;




method_call_expr_opt 
: expr list_method_call_expr

|
;




list_method_call_expr 
: ',' expr list_method_call_expr

|
;




location 
: ID 
{
    Symbole * sym = lookup(SYMTABLE, $1);
    if (sym == NULL) {
        fprintf(stderr, "ID \"%s\" is not declared\n", $1);
        exit(1);
    }
    free($1);
    $$.ptr = sym;
}

| ID '[' expr ']'
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
    if (!(  (($1.ptr)->type == CONST_INT || ($1.ptr)->type == VAR_INT)
        && (($3.ptr)->type == CONST_INT || ($3.ptr)->type == VAR_INT))) {
        fprintf(stderr, "ADD type not INT\n");
        exit(1);
    };
    Symbole * sym = newtemp(SYMTABLE);
    sym->type = VAR_INT;
    gencode(CODE, OP_ADD, sym, $1.ptr, $3.ptr);
    $$.ptr = sym;
    SymboleTableRoot_dump(SYMTABLE);
}

| expr '-' expr 
{
    if (!(  (($1.ptr)->type == CONST_INT || ($1.ptr)->type == VAR_INT)
        && (($3.ptr)->type == CONST_INT || ($3.ptr)->type == VAR_INT))) {
        fprintf(stderr, "SUB type not INT\n");
        exit(1);
    };
    Symbole * sym = newtemp(SYMTABLE);
    sym->type = VAR_INT;
    gencode(CODE, OP_SUB, sym, $1.ptr, $3.ptr);
    $$.ptr = sym;
}

| expr '*' expr
{
    if (!(  (($1.ptr)->type == CONST_INT || ($1.ptr)->type == VAR_INT)
        && (($3.ptr)->type == CONST_INT || ($3.ptr)->type == VAR_INT))) {
        fprintf(stderr, "MUL type not INT\n");
        exit(1);
    };
    Symbole * sym = newtemp(SYMTABLE);
    sym->type = VAR_INT;
    gencode(CODE, OP_MUL, sym, $1.ptr, $3.ptr);
    $$.ptr = sym;
}

| expr '/' expr
{
    if (!(  (($1.ptr)->type == CONST_INT || ($1.ptr)->type == VAR_INT)
        && (($3.ptr)->type == CONST_INT || ($3.ptr)->type == VAR_INT))) {
        fprintf(stderr, "DIV type not INT\n");
        exit(1);
    };
    Symbole * sym = newtemp(SYMTABLE);
    sym->type = VAR_INT;
    gencode(CODE, OP_DIV, sym, $1.ptr, $3.ptr);
    $$.ptr = sym;
}

| expr '%' expr
{
    if (!(  (($1.ptr)->type == CONST_INT || ($1.ptr)->type == VAR_INT)
        && (($3.ptr)->type == CONST_INT || ($3.ptr)->type == VAR_INT))) {
        fprintf(stderr, "MOD type not INT\n");
        exit(1);
    };
    Symbole * sym = newtemp(SYMTABLE);
    sym->type = VAR_INT;
    gencode(CODE, OP_MOD, sym, $1.ptr, $3.ptr);
    $$.ptr = sym;
}

| expr '<' expr
{
    if (!(  (($1.ptr)->type == CONST_INT || ($1.ptr)->type == VAR_INT)
        && (($3.ptr)->type == CONST_INT || ($3.ptr)->type == VAR_INT))) {
        fprintf(stderr, "LT type not INT\n");
        exit(1);
    };
    Symbole * sym = newtemp(SYMTABLE);
    sym->type = VAR_BOOL;
    gencode(CODE, OP_LT, sym, $1.ptr, $3.ptr);
    $$.ptr = sym;
}

| expr '>' expr
{
    if (!(  (($1.ptr)->type == CONST_INT || ($1.ptr)->type == VAR_INT)
        && (($3.ptr)->type == CONST_INT || ($3.ptr)->type == VAR_INT))) {
        fprintf(stderr, "GT type not INT\n");
        exit(1);
    };
    Symbole * sym = newtemp(SYMTABLE);
    sym->type = VAR_BOOL;
    gencode(CODE, OP_GT, sym, $1.ptr, $3.ptr);
    $$.ptr = sym;
}

| expr LE expr
{
    if (!(  (($1.ptr)->type == CONST_INT || ($1.ptr)->type == VAR_INT)
        && (($3.ptr)->type == CONST_INT || ($3.ptr)->type == VAR_INT))) {
        fprintf(stderr, "LE type not INT\n");
        exit(1);
    };
    Symbole * sym = newtemp(SYMTABLE);
    sym->type = VAR_BOOL;
    gencode(CODE, OP_LE, sym, $1.ptr, $3.ptr);
    $$.ptr = sym;
}

| expr GE expr
{
    if (!(  (($1.ptr)->type == CONST_INT || ($1.ptr)->type == VAR_INT)
        && (($3.ptr)->type == CONST_INT || ($3.ptr)->type == VAR_INT))) {
        fprintf(stderr, "GE type not INT\n");
        exit(1);
    };
    Symbole * sym = newtemp(SYMTABLE);
    sym->type = VAR_BOOL;
    gencode(CODE, OP_GE, sym, $1.ptr, $3.ptr);
    $$.ptr = sym;
}

| expr EQ expr
{
    if (!(
           ((($1.ptr)->type == CONST_INT || ($1.ptr)->type == VAR_INT) && (($3.ptr)->type == CONST_INT || ($3.ptr)->type == VAR_INT)) 
        || ((($1.ptr)->type == CONST_BOOL || ($1.ptr)->type == VAR_BOOL) && (($3.ptr)->type == CONST_BOOL || ($3.ptr)->type == VAR_BOOL)) 
       )) {
        fprintf(stderr, "EQ type not INT or BOOL\n");
        exit(1);
    };
    Symbole * sym = newtemp(SYMTABLE);
    sym->type = VAR_BOOL;
    gencode(CODE, OP_EQ, sym, $1.ptr, $3.ptr);
    $$.ptr = sym;
}

| expr NE expr
{
    if (!(
           ((($1.ptr)->type == CONST_INT || ($1.ptr)->type == VAR_INT) && (($3.ptr)->type == CONST_INT || ($3.ptr)->type == VAR_INT)) 
        || ((($1.ptr)->type == CONST_BOOL || ($1.ptr)->type == VAR_BOOL) && (($3.ptr)->type == CONST_BOOL || ($3.ptr)->type == VAR_BOOL)) 
       )) {
        fprintf(stderr, "NE type not INT or BOOL\n");
        exit(1);
    };
    Symbole * sym = newtemp(SYMTABLE);
    sym->type = VAR_BOOL;
    gencode(CODE, OP_NE, sym, $1.ptr, $3.ptr);
    $$.ptr = sym;
}

| expr AND expr
{
    if (!(  (($1.ptr)->type == CONST_BOOL || ($1.ptr)->type == VAR_BOOL)
        && (($3.ptr)->type == CONST_BOOL || ($3.ptr)->type == VAR_BOOL))) {
        fprintf(stderr, "AND type not BOOL\n");
        exit(1);
    };
    Symbole * sym = newtemp(SYMTABLE);
    sym->type = VAR_BOOL;
    gencode(CODE, OP_AND, sym, $1.ptr, $3.ptr);
    $$.ptr = sym;
}

| expr OR expr
{
    if (!(  (($1.ptr)->type == CONST_BOOL || ($1.ptr)->type == VAR_BOOL)
        && (($3.ptr)->type == CONST_BOOL || ($3.ptr)->type == VAR_BOOL))) {
        fprintf(stderr, "OR type not BOOL\n");
        exit(1);
    };
    Symbole * sym = newtemp(SYMTABLE);
    sym->type = VAR_BOOL;
    gencode(CODE, OP_OR, sym, $1.ptr, $3.ptr);
    $$.ptr = sym;
}

| '!' expr
{
    if (!(  ($2.ptr)->type == CONST_BOOL || ($2.ptr)->type == VAR_BOOL)
        ) {
        fprintf(stderr, "NOT type not BOOL\n");
        exit(1);
    };
    Symbole * sym = newtemp(SYMTABLE);
    sym->type = VAR_BOOL;
    gencode(CODE, OP_NOT, sym, $2.ptr, NULL);
    $$.ptr = sym;
}

| '-' expr 
{
	if (!(  ($2.ptr)->type == CONST_INT || ($2.ptr)->type == VAR_INT)
        ) {
        fprintf(stderr, "UMOINS type not INT\n");
        exit(1);
    };
    Symbole * sym = newtemp(SYMTABLE);
    sym->type = VAR_INT;
    gencode(CODE, OP_UMOINS, sym, $2.ptr, NULL);
} %prec UMOINS

| '(' expr ')'
{
    $$.ptr = $2.ptr;
}
;  




literal : INT_LIT 
{
    char name[20];
    int res = snprintf(name, 20, "-I%li", $1);
    if (res < 0 || res >= 20) {
        perror("int_lit snprintf");
        exit(1);
    }
    Symbole * sym = newconst(SYMTABLE, name);
    sym->type = CONST_INT;
    sym->value.int_lit = $1;
    $$.ptr = sym;
}

| CHAR_LIT 
{
    char name[20];
    int res = snprintf(name, 20, "-C%li", $1);
    if (res < 0 || res >= 20) {
        perror("char_lit snprintf");
        exit(1);
    }
    Symbole * sym = newconst(SYMTABLE, name);
    sym->type = CONST_CHAR;
    sym->value.char_lit = $1;
    $$.ptr = sym;
}

| BOOL_LIT 
{
    char name[20];
    int res = snprintf(name, 20, "-B%li", $1);
    if (res < 0 || res >= 20) {
        perror("bool_lit snprintf");
        exit(1);
    }
    Symbole * sym = newconst(SYMTABLE, name);
    sym->type = CONST_BOOL;
    sym->value.bool_lit = $1;
    $$.ptr = sym;
}
;

%%



int yyerror(char* s) {
    printf("error : %s\n", s);
    return 1;
}
