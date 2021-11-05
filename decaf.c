#include <stdio.h>
#include <stdint.h>

extern int yyparse(void);

int main(void) {
    int res = yyparse();
    return res;
}