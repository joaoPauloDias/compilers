#include "asd.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

AsdTree *asd_new(const char *label)
{
    AsdTree *ret = malloc(sizeof(AsdTree));
    assert(ret != NULL);

    ret->label = strdup(label);
    ret->number_of_children = 0;
    ret->children = NULL;

    return ret;
}

AsdTree *asd_with_capacity(const char *label, uint16_t capacity)
{
    AsdTree *ret = asd_new(label);

    ret->children = malloc(capacity);
    ret->number_of_children = capacity;

    return ret;
}

void asd_free(AsdTree *tree)
{
    if (tree != NULL)
    {
        int i;
        for (i = 0; i < tree->number_of_children; i++)
        {
            asd_free(tree->children[i]);
        }
        free(tree->children);
        free(tree->label);
        free(tree);
    }
    else
    {
        printf("Erro: %s recebeu parâmetro tree = %p.\n", __FUNCTION__, tree);
    }
}

void asd_add_child(AsdTree *tree, AsdTree *child)
{
    if (tree != NULL && child != NULL)
    {
        tree->number_of_children++;
        tree->children = realloc(tree->children, tree->number_of_children * sizeof(AsdTree *));
        tree->children[tree->number_of_children - 1] = child;
    }
    else
    {
        printf("Erro: %s recebeu parâmetro tree = %p / %p.\n", __FUNCTION__, tree, child);
    }
}

static void _asd_print(FILE *foutput, AsdTree *tree, int profundidade)
{
    int i;
    if (tree != NULL)
    {
        fprintf(foutput, "%d%*s: Nó '%s' tem %hu filhos:\n", profundidade, profundidade * 2, "", tree->label,
                tree->number_of_children);
        for (i = 0; i < tree->number_of_children; i++)
        {
            _asd_print(foutput, tree->children[i], profundidade + 1);
        }
    }
    else
    {
        printf("Erro: %s recebeu parâmetro tree = %p.\n", __FUNCTION__, tree);
    }
}

void asd_print(AsdTree *tree)
{
    FILE *foutput = stderr;
    if (tree != NULL)
    {
        _asd_print(foutput, tree, 0);
    }
    else
    {
        printf("Erro: %s recebeu parâmetro tree = %p.\n", __FUNCTION__, tree);
    }
}

static void _asd_print_graphviz(FILE *foutput, AsdTree *tree)
{
    int i;
    if (tree != NULL)
    {
        fprintf(foutput, "  %ld [ label=\"%s\" ];\n", (long)tree, tree->label);
        for (i = 0; i < tree->number_of_children; i++)
        {
            fprintf(foutput, "  %ld -> %ld;\n", (long)tree, (long)tree->children[i]);
            _asd_print_graphviz(foutput, tree->children[i]);
        }
    }
    else
    {
        printf("Erro: %s recebeu parâmetro tree = %p.\n", __FUNCTION__, tree);
    }
}

void asd_print_graphviz(AsdTree *tree)
{
    FILE *foutput = stdout;
    if (tree != NULL)
    {
        fprintf(foutput, "digraph grafo {\n");
        _asd_print_graphviz(foutput, tree);
        fprintf(foutput, "}\n");
    }
    else
    {
        printf("Erro: %s recebeu parâmetro tree = %p.\n", __FUNCTION__, tree);
    }
}
