%{
#include <stdio.h>
extern int yylex();
int yyerror(char*);

%}

%token COMMENT
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
%token PROGRAM
%token GE
%token LE
%token EQ
%token NE
%token SUB
%token ADD
%token AND
%token OR

%token STRING_LIT
%token CHAR_LIT
%token BOOL_LIT
%token INT_LIT

%token ID

%left OR
%left AND
%left EQ NE
%left '<' '>' LE GE
%left '+' '-'
%left '*' '/' '%'
%left '!'
%left UMOINS

%%

program : CLASS PROGRAM '{' list_field_decl list_method_decl '}'
        ;




list_field_decl : field_decl list_field_decl
                |
                ;

field_decl : type field list_field ';'

list_field : ',' field list_field
           |
           ;
           
field : ID 
      | ID '[' INT_LIT ']'
      ;




list_method_decl : method_decl list_method_decl
                 |
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
          | IF '(' expr ')' block statement_else_opt     
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
     | expr bin_op expr
     | '-' expr         %prec UMOINS
     | '!' expr
     | '(' expr ')'
     ;
     
     

     
bin_op : arith_op 
       | rel_op
       | eq_op
       | cond_op 
       ;

    
    
    
arith_op : '+'
         | '-'
         | '*'
         | '/'
         | '%'
         ;





 rel_op : '<'
        | '>'
        | LE
        | GE
        ;




eq_op : EQ
      | NE
      ;




cond_op : AND
        | OR
        ;




literal : INT_LIT
        | CHAR_LIT
        | BOOL_LIT
        ;




%%



int yyerror(char* s) {
    printf("error : %s\n", s);
    return 1;
}
