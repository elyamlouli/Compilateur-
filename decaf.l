%option noyywrap

%{
#include <ctype.h>
#include <stdio.h>
#include "decaf.tab.h"
%}

comment "//"(.)*"\n"
alpha [a-zA-Z_]
digit [0-9]
char [ -!#-&(-\[\]-~]

string_literal \"{char}*\"
char_literal \'{char}\'
bool_literal true|false
hex_literal "Ox"{hex_digit}{hex_digit}*
decimal_literal {digit}{digit}*
int_literal {decimal_literal}|{hex_literal}
hex_digit {digit}|[a-f]|[A-F]
alpha_num {alpha}|{digit}
id {alpha}{alpha_num}*


%%
{comment} { return COMMENT; }

 /* Mots réservés */
"boolean" { return BOOLEAN; }
"break" { return BREAK; }
"class" {return CLASS;}
"continue" {return CONTINUE;}
"else" {return ELSE;}
"for" {return FOR;}
"if" { return IF; }
"int" { return INT; }
"return" { return RETURN; }
"void" { return VOID; }

 /* Fin des mots réservés */
"Program" {return PROGRAM;}

"+"|"-"|"*"|"/"|"%" { return yytext[0]; }

";" { return yytext[0]; }

"{"|"}" { return yytext[0]; }
"["|"]" { return yytext[0]; }
"("|")" { return yytext[0]; }


"<"|">" { return yytext[0]; }
"<="  { return LE; }
">="  { return GE; }
"==" { return EQ; }
"!=" { return NE; }

"=" { return yytext[0]; }
"-=" { return SUB; }
"+=" { return ADD; }

"&&" { return AND; }
"||" { return OR; }
"!" { return yytext[0]; }

{string_literal} {return STRING_LIT;}
{char_literal} {return CHAR_LIT;}
{bool_literal} {return BOOL_LIT;}
{int_literal} {return INT_LIT;}



{id} { return ID; }
[[:space:]] ;

%%