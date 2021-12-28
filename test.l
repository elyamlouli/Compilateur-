%{
#include <stdio.h>
%}

char [ -!#-&(-\[\]-~]|"\\\\"|"\\'"|"\\t"|"\\n"|"\\\""
string_literal \"{char}*\"


%%
 // [ -!#-&(-\[\]-~]|"\\"|"\'"|"\t"|"\n"|"\"" {puts("aaa");}
{string_literal} { printf("string : %s\n", yytext); fflush(stdout); }

[[:space:]] ;
. { printf("error\n"); fflush(stdout); }
%%
