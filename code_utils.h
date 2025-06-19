#pragma once

#include "asd.h"

#define MAX_TEMPORARY_LENGTH 128
#define MAX_LABEL_LENGTH 128

static int current_temporary = 0;
static int current_label = 0;

typedef struct iloc_t
{
    char *mnemonic;
    char *label;
    char *arg1;
    char *arg2;
    char *arg3;
} iloc_t;

typedef struct code_t
{
    struct code_t *next;
    struct iloc_t instruction;
} code_t;

code_t *generate_code(char *mnemonic, const char *arg1, const char *arg2, const char *arg3);
char *generate_label();
char *generate_temporary();
void generate_code_binary_operation(char *mnemonic, AsdTree *root, struct AsdTree *left, struct AsdTree *right);
code_t *concatenate_code(code_t *first, code_t *second);
code_t *concatenate_multiple_codes(code_t *first, ...);
void print_code(code_t *code);
