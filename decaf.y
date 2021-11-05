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
%token GT
%token LE
%token LT
%token EQ
%token NE
%token SUB
%token ADD
%token AND
%token OR

%token STRING_LIT
%token CHAR_LIT
%token BOOL_LIT
%token HEX_LIT
%token DECIMAL_LIT
%token INT_LIT

%token ID

%%

S : ;

%%


int yyerror(char* s) {
    printf("error : %s\n", s);
    return 1;
}
