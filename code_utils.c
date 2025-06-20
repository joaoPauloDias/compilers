#include "code_utils.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "arena.h"

code_t *generate_code(char *mnemonic, const char *arg1, const char *arg2, const char *arg3)
{
    static const code_t zero = {};
    code_t *code = arena_alloc(allocator, sizeof(code_t));
    *code = zero;

    code->instruction.mnemonic = mnemonic;

    if (arg1 != NULL)
    {
        code->instruction.arg1 = arg1;
    }
    if (arg2 != NULL)
    {
        code->instruction.arg2 = arg2;
    }
    if (arg3 != NULL)
    {
        code->instruction.arg3 = arg3;
    }

    return code;
}

char *generate_temporary()
{
    static uint32_t current_temporary = 0;
    char temp[MAX_TEMPORARY_LENGTH];
    snprintf(temp, MAX_TEMPORARY_LENGTH, "r%" PRIu32, current_temporary++);
    return arena_strdup(allocator, temp);
}

char *generate_label()
{
    static uint32_t current_label = 0;
    char label[MAX_LABEL_LENGTH];
    snprintf(label, MAX_LABEL_LENGTH, "L%" PRIu32, current_label++);
    return arena_strdup(allocator, label);
}

void generate_code_binary_operation(char *mnemonic, AsdTree *root, AsdTree *left, AsdTree *right)
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

    if (first == NULL)
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
    if (tail != NULL)
    {
        while (tail->next != NULL)
        {
            tail = tail->next;
        }
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
        while (tail->next != NULL)
        {
            tail = tail->next;
        }
    }

    va_end(ptr);
    return first;
}

void print_code(const code_t *code)
{
    if (code == NULL)
    {
        return;
    }

    const iloc_t *inst = &code->instruction;
    const char *label = inst->label ? inst->label : "";
    const char *colon = inst->label ? ":" : "";
    const char *arg1 = inst->arg1 ? inst->arg1 : "";
    const char *arg2 = inst->arg2 ? inst->arg2 : "";
    const char *arg3 = inst->arg3 ? inst->arg3 : "";

    if (strcmp(inst->mnemonic, "cbr") == 0 || strcmp(inst->mnemonic, "jumpI") == 0)
    {
        printf("%2s%1s %-8s %3s -> %3s%1s %3s\n", label, colon, inst->mnemonic, arg1, arg2, inst->arg3 ? "," : "",
               arg3);
    }
    else if (strcmp(inst->mnemonic, "storeAI") == 0)
    {
        printf("%2s%1s %-8s %3s => %3s%1s %3s\n", label, colon, inst->mnemonic, arg1, arg2, inst->arg2 ? "," : "",
               arg3);
    }
    else if (strcmp(inst->mnemonic, "nop") == 0)
    {
        printf("%2s%1s %-8s\n", label, colon, inst->mnemonic);
    }
    else
    {
        const char *arrow = (strncmp(inst->mnemonic, "cmp", 3) != 0) ? "=>" : "->";
        printf("%2s%1s %-8s %3s%1s %3s %s %3s\n", label, colon, inst->mnemonic, arg1, inst->arg2 ? "," : "", arg2,
               arrow, arg3);
    }

    print_code(code->next);
}
