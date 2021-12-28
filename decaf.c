#include <stdio.h>
#include <stdint.h>

#include "decaf.h"

extern int yyparse(void);

SymboleTableRoot * SYMTABLE;

int main(void) {
    SYMTABLE = SymboleTableRoot_new();
    int res = yyparse();
    SymboleTableRoot_free(SYMTABLE);
    return res;
}