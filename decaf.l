%option noyywrap

%{
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "decaf.tab.h"

extern int yyerror();
extern int yyleng;

#define CHECKMALLOC(op) if (op == NULL) { perror(#op); exit(1); }
%}

comment "//"(.)*"\n"
alpha [a-zA-Z_]
digit [0-9]
char [ -!#-&(-\[\]-~]|"\\\\"|"\\'"|"\\t"|"\\n"|"\\\""

string_literal \"{char}*\"
char_literal \'{char}\'
bool_literal true|false
hex_literal "0x"{hex_digit}{hex_digit}*
decimal_literal {digit}{digit}*
 // int_literal {decimal_literal}|{hex_literal}
hex_digit {digit}|[a-f]|[A-F]
alpha_num {alpha}|{digit}
id {alpha}{alpha_num}*


%%
{comment} ;

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

"main" { return MAIN; }

 /* Fin des mots réservés */

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
"-=" { return DECR; }
"+=" { return INCR; }

"&&" { return AND; }
"||" { return OR; }
"!" { return yytext[0]; }

{string_literal} { 
    yylval.strval = strndup(yytext, yyleng);
    CHECKMALLOC(yylval.strval);
    return STRING_LIT;
}

{char_literal} {
    if (yytext[0] == '\\') {
        if (yytext[1] == 't') {
            yylval.intval = '\t';
        } else if (yytext[1] == 'n') {
            yylval.intval == 'n';
        } else if (yytext[1] == '\'') {
            yylval.intval = '\'';
        } else if (yytext[1] == '"') {
            yylval.intval = '"';
        } else if (yytext[1] == '\\') {
            yylval.intval = '\\';
        }
    } else {
        yylval.intval = yytext[0];
    }
    return CHAR_LIT;
}

{bool_literal} {
    yylval.intval = (yytext[0] == 't');
    return BOOL_LIT;
}

{decimal_literal} {
    if (strlen(yytext) > 10) {
        yyerror();
        return YYerror;
    }
    sscanf(yytext,"%li",&(yylval.intval));
    return INT_LIT;
}

{hex_literal} {
    if (strlen(yytext) > 10) {
        yyerror();
        return YYerror;
    }
    sscanf(yytext,"%x",&(yylval.intval));
    return INT_LIT;
}

"," { return yytext[0]; }

"WriteString" { return WS; }

{id} { 
    yylval.strval = strndup(yytext, yyleng);
    CHECKMALLOC(yylval.strval);
    return ID; 
}

[[:space:]] ;

. { puts("error"); }

%%

