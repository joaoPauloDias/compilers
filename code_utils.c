#include "code_utils.h"
#include "symbol_table.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

code_t *generate_code(char *mnemonic, const char *arg1, const char *arg2, const char *arg3)
{
    code_t *code = (code_t *)malloc(sizeof(code_t));

    code->instruction.mnemonic = mnemonic;

    if (arg1 != NULL)
    {
        code->instruction.arg1 = strdup(arg1);
    }
    if (arg2 != NULL)
    {
        code->instruction.arg2 = strdup(arg2);
    }
    if (arg3 != NULL)
    {
        code->instruction.arg3 = strdup(arg3);
    }

    code->next = NULL;

    return code;
}

char *generate_temporary()
{
    char temp[MAX_TEMPORARY_LENGTH];
    snprintf(temp, MAX_TEMPORARY_LENGTH, "r%d", current_temporary++);
    return strdup(temp);
}

char *generate_label()
{
    char label[MAX_LABEL_LENGTH];
    snprintf(label, MAX_LABEL_LENGTH, "L%d", current_label++);
    return strdup(label);
}

void generate_code_binary_operation(char *mnemonic, AsdTree *root, struct AsdTree *left, struct AsdTree *right)
{
    root->location = generate_temporary();
    root->code = concatenate_multiple_codes(
        left->code, right->code, generate_code(mnemonic, left->location, right->location, root->location), NULL);
}

code_t *concatenate_code(code_t *first, code_t *second)
{
    if (second == NULL)
    {
        return first;
    }

    if (first == NULL && second != NULL)
    {
        return second;
    }

    code_t *current_code = first;

    while (current_code->next != NULL)
    {
        current_code = current_code->next;
    }

    current_code->next = second;

    return first;
}

code_t *concatenate_multiple_codes(code_t *first, ...)
{
    va_list ptr;
    va_start(ptr, first);

    code_t *tail = first;
    if (tail)
    {
        while (tail->next)
            tail = tail->next;
    }

    code_t *next_code;
    while ((next_code = va_arg(ptr, code_t *)) != NULL)
    {
        if (!first)
        {
            first = tail = next_code;
        }
        else
        {
            tail->next = next_code;
        }
        while (tail->next)
            tail = tail->next;
    }

    va_end(ptr);
    return first;
}

void print_code(code_t *code)
{
    if (code == NULL)
    {
        return;
    }

    struct iloc_t instruction = code->instruction;

    if (strcmp(instruction.mnemonic, "cbr") == 0 || strcmp(instruction.mnemonic, "jumpI") == 0)
    {
        printf("%2s%1s %-8s %3s -> %3s%1s %3s\n", instruction.label != NULL ? instruction.label : "",
               instruction.label != NULL ? ":" : "", instruction.mnemonic,
               instruction.arg1 != NULL ? instruction.arg1 : "", instruction.arg2 != NULL ? instruction.arg2 : "",
               instruction.arg3 != NULL ? "," : "", instruction.arg3 != NULL ? instruction.arg3 : "");
    }
    else if (strcmp(instruction.mnemonic, "storeAI") == 0)
    {
        printf("%2s%1s %-8s %3s => %3s%1s %3s\n", instruction.label != NULL ? instruction.label : "",
               instruction.label != NULL ? ":" : "", instruction.mnemonic,
               instruction.arg1 != NULL ? instruction.arg1 : "", instruction.arg2 != NULL ? instruction.arg2 : "",
               instruction.arg2 != NULL ? "," : "", instruction.arg3 != NULL ? instruction.arg3 : "");
    }
    else if (strcmp(instruction.mnemonic, "nop") == 0)
    {
        printf("%2s%1s %-8s\n", instruction.label != NULL ? instruction.label : "",
               instruction.label != NULL ? ":" : "", instruction.mnemonic);
    }
    else
    {
        char *arrow = strncmp(instruction.mnemonic, "cmp", 3) != 0 ? "=>" : "->";
        printf("%2s%1s %-8s %3s%1s %3s %s %3s\n", instruction.label != NULL ? instruction.label : "",
               instruction.label != NULL ? ":" : "", instruction.mnemonic,
               instruction.arg1 != NULL ? instruction.arg1 : "", instruction.arg2 != NULL ? "," : "",
               instruction.arg2 != NULL ? instruction.arg2 : "", arrow,
               instruction.arg3 != NULL ? instruction.arg3 : "");
    }

    print_code(code->next);
}
