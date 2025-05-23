#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "types.h"

typedef enum
{
    SyLiteral,
    SyIdentifier,
    SyFunction,
} SymbolNature;

typedef struct
{
    const char *key;
    Type type;
    SymbolNature nature;
    // Only != 0 if it is a function
    size_t num_args;
    Type* args;
} SymbolEntry;

typedef struct
{
    size_t count;
    size_t capacity;
    SymbolEntry *items;
} SymbolTable;

typedef struct
{
    size_t count;
    size_t capacity;
    size_t *items;
} Scopes;

typedef struct
{
    size_t count;
    size_t capacity;
    struct
    {
        SymbolEntry* function;
        size_t count;
    }* items;
} ParamStack;

enum ParamResult
{
    Match,
    NoMatch,
    Overflow,
};

extern SymbolTable table;

extern Scopes scopes;

extern ParamStack param_stack;

/// Returns `true` on success
bool push_if_not_declared(SymbolEntry entry);

void push_symbol(SymbolEntry entry);

SymbolEntry* top_symbol();

void push_scope();

void pop_scope();

size_t top_scope();

bool contains_key(const char *key);

bool contains_in_current_scope(const char *key);

// Returns null if does not contain
SymbolEntry* get_entry(const char* key);

enum ParamResult check_param(Type type);
