#include <stdio.h>
#include <stdint.h>

#include "decaf.h"

extern int yyparse(void);

SymboleTableRoot * SYMTABLE;
Code * CODE;
FunctionsContexts * FUN_CTX;

int main(void) {
    SYMTABLE = SymboleTableRoot_new();
    CODE = Code_new();
    FUN_CTX = FunctionsContexts_new();
    FILE * file = fopen("decaf.mips", "w+");
    if (file == NULL) {
        perror("fopen");
        exit(1);
    }
    // FILE * file = stdout;

    int res = yyparse();
    genMIPS(file, CODE, SYMTABLE, FUN_CTX);
    fflush(file);
    if (fclose(file) == EOF) {
        perror("fclose");
        exit(1);
    }
    FunctionsContexts_free(FUN_CTX);
    Code_free(CODE);
    SymboleTableRoot_free(SYMTABLE);
    return res;
}