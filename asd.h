#pragma once

#include <stdint.h>

typedef enum
{
    TKLiteral,
    TKIdentifier,
} TokenKind;

typedef struct
{
    uint32_t line_number;
    TokenKind kind;
    char *lexem;
} LexicalValue;

typedef struct AsdTree
{
    const char *label;
    uint16_t number_of_children;
    struct AsdTree **children;
} AsdTree;

/*
 * Função asd_new, cria um nó sem filhos com o label informado.
 */
AsdTree *asd_new(const char *label);

AsdTree *asd_with_capacity(const char *label, uint16_t capacity);

/*
 * Função asd_tree, libera recursivamente o nó e seus filhos.
 */
void asd_free(AsdTree *tree);

/*
 * Função asd_add_child, adiciona child como filho de tree.
 */
void asd_add_child(AsdTree *tree, AsdTree *child);

/*
 * Função asd_print, imprime recursivamente a árvore.
 */
void asd_print(AsdTree *tree);

/*
 * Função asd_print_graphviz, idem, em formato DOT
 */
void asd_print_graphviz(AsdTree *tree);
