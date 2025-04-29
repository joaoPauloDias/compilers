#include "asd.h"
#include <stdio.h>

#include "parser.tab.h" //arquivo gerado com bison -d parser.y
                        //inclua tal comando no teu workflow (Makefile)

extern int yyparse(void);
extern int yylex_destroy(void);

AsdTree *arvore = NULL;

int main(int argc, char **argv)
{
    int ret = yyparse();

    asd_print_graphviz(arvore);
    asd_free(arvore);
    yylex_destroy();

    return ret;
}
