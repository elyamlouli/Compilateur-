#include <stdio.h>
#include <stdint.h>

#include "decaf.h"

extern int yyparse(void);

int main(void) {
    symtable = SymboleTableRoot_new();
    int res = yyparse();
    return res;
}