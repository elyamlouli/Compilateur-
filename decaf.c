#include <stdio.h>
#include <stdint.h>

#include "decaf.h"

extern int yyparse(void);

SymboleTableRoot * SYMTABLE;
Code * CODE;

int main(void) {
    SYMTABLE = SymboleTableRoot_new();
    CODE = Code_new();
    int res = yyparse();
    Code_free(CODE);
    SymboleTableRoot_free(SYMTABLE);
    return res;
}