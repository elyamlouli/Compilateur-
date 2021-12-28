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
%token SUB
%token ADD
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

%token ID


%type <exprval> literal

%left OR
%left AND
%left EQ NE
%left '<' '>' LE GE
%left '+' '-'
%left '*' '/' '%'
%left '!'
%left UMOINS

%%

program : CLASS ID '{' field_method '}'
        ;


field_method : field_decl field_method
             | VOID ID '(' method_arg_opt ')' block list_method_decl
             | type ID '(' method_arg_opt ')' block list_method_decl
             | VOID MAIN '(' ')' block
             ;

field_decl : type field list_field ';'

list_field : ',' field list_field
           |
           ;
           
field : ID 
      | ID '[' INT_LIT ']'
      ;




list_method_decl : method_decl list_method_decl
                 | VOID MAIN '(' ')' block 
                 ;

method_decl : method_type ID '(' method_arg_opt ')' block
            ;

method_type : type 
            | VOID
            ;

method_arg_opt : method_arg list_method_arg
               |
               ;

list_method_arg : ',' method_arg list_method_arg
                |
                ;

method_arg : type ID
           ;




block : '{' list_var_decl list_statement '}'

list_var_decl : var_decl list_var_decl
             |
             ;

var_decl : type ID list_id ';'

list_id : ',' ID list_id
        |
        ;


type : INT
     | BOOLEAN
     ;




list_statement : statement list_statement
               |
               ;

statement : location assign_op expr ';'
          | method_call ';'
          | IF '(' BOOL_LIT ')' block statement_else_opt
          | FOR ID '=' expr ',' expr block
          | RETURN statement_expr_opt ';'
          | BREAK ';'
          | CONTINUE ';'
          | block
          ;

statement_else_opt : ELSE block
                   |
                   ;

statement_expr_opt : expr
                   |
                   ;
                   



assign_op : '='
           | ADD
           | SUB
           ;




method_call : method_name '(' method_call_expr_opt ')'
            | WS '(' STRING_LIT ')'
            ;

method_name : ID
            ;

method_call_expr_opt : expr list_method_call_expr
                     |
                     ;

list_method_call_expr : ',' expr list_method_call_expr
                      |
                      ;




location : ID 
         | ID '[' expr ']'


         

expr : location
     | method_call
     | literal
     | INT_LIT '+' INT_LIT
     | expr '-' expr
     | expr '*' expr
     | expr '/' expr
     | expr '%' expr
     | expr '<' expr
     | expr '>' expr
     | expr LE expr
     | expr GE expr
     | expr EQ expr
     | expr NE expr
     | expr AND expr
     | expr OR expr
     | '-' expr         %prec UMOINS
     | '!' expr
     | '(' expr ')'
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
              sym->type = INT_LIT;
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
              sym->type = CHAR_LIT;
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
              sym->type = BOOL_LIT;
              sym->value.bool_lit = $1;
              $$.ptr = sym;
          }
        ;




%%



int yyerror(char* s) {
    printf("error : %s\n", s);
    return 1;
}
