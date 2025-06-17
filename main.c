#include <stdio.h>

#include "arena.h"
#include "asd.h"
#include "symbol_table.h"

#include "parser.tab.h"

extern int yyparse(void);
extern int yylex_destroy(void);

AsdTree *arvore = NULL;
Arena *allocator = NULL;
SymbolTable table = {};
Scopes scopes = {};
ParamStack param_stack = {};

int main(int argc, char **argv)
{
    Arena arena = {};
    allocator = &arena;

    table = (SymbolTable){.capacity = 128, .items = arena_alloc(allocator, sizeof(SymbolEntry) * 128)};

    int ret = yyparse();

    // asd_print_graphviz(arvore);
    print_code(arvore->code);
    asd_free(arvore);
    yylex_destroy();

    arena_free(allocator);

    return ret;
}
