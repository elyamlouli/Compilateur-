#include <stdio.h>
#include <stdint.h>
#include <getopt.h>

#include "decaf.h"

extern int yyparse(void);

SymboleTableRoot * SYMTABLE;
Code * CODE;
FunctionsContexts * FUN_CTX;


void print_usage() {
    fprintf(stdout,"\nUsage :\t./decaf [-h] [-version] [-tos] [-o <name>]\n"
    "\t-h pour help\n"
    "\t-version afficher les membres du projet\n"
    "\t-tos pour afficher la table des symboles\n"
    "\t-o <name> pour écrire le code résultat dans le fichier name\n"
    "\n");
}

void print_symtable() {
    SymboleTableRoot_dump(SYMTABLE);
}


int main(int argc, char **argv) {

    char* file_output = NULL; 

    // traitement arguments
    int opt= 0;
    static struct option long_options[] = {
        {"version", no_argument, 0,'v' },
        {"tos", no_argument, 0,'t' }
    };

    int dump_symtable = 0;
    int long_index = 0;
    while ((opt = getopt_long_only(argc, argv,"hvto:",long_options, &long_index )) != -1) {
        switch (opt) {
            case 'h':
                print_usage();
                exit(EXIT_SUCCESS);
            case 'v' : 
                printf("Michel TCHING, Alexandre SEGOND, Fatima Zahra EL YAMLOULI\n");
                break;
            case 't' : 
                dump_symtable = 1;
                break;
            case 'o': 
                file_output = optarg; 
                break;
            default: 
                print_usage(); 
                exit(EXIT_FAILURE);
        }
    }
    // fin arguments

    SYMTABLE = SymboleTableRoot_new();
    CODE = Code_new();
    FUN_CTX = FunctionsContexts_new();
    FILE * file = stdout;
    // FILE * file = fopen("decaf.mips", "w+");

    if (file_output != NULL) {
        file = fopen(file_output, "w+");
        if (file == NULL) {
            perror("fopen");
            exit(EXIT_FAILURE);
        }
    }
    
    
    int res = yyparse();
    genMIPS(file, CODE, SYMTABLE, FUN_CTX);
    fflush(file);
    if (fclose(file) == EOF) {
        perror("fclose");
        exit(EXIT_FAILURE);
    }

    if (dump_symtable) {
        print_symtable();
    }

    FunctionsContexts_free(FUN_CTX);
    Code_free(CODE);
    SymboleTableRoot_free(SYMTABLE);
    return res;
}






