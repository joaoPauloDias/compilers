#include "symbol_table.h"

#include "arena.h"

#include <string.h>

void push_symbol(SymbolEntry entry)
{
    SymbolEntry *current_top_symbol = top_symbol();
    if (current_top_symbol != NULL)
    {
        entry.offset = current_top_symbol->offset + 4;
    }
    else
    {
        entry.offset = table.base_offset;
    }
    entry.is_global = top_scope() == 0;
    arena_da_append(allocator, &table, entry);
}

SymbolEntry *top_symbol()
{
    if (table.count == 0)
    {
        return NULL;
    }
    return &table.items[table.count - 1];
}

void push_scope()
{
    if (table.count > 0)
    {
        table.base_offset = top_symbol()->offset + 4;
    }
    else
    {
        table.base_offset = 0;
    }
    arena_da_append(allocator, &scopes, table.count);
}

void pop_scope()
{
    table.count = scopes.items[scopes.count - 1];
    scopes.count -= 1;
    if (scopes.count == 0 || table.count == 0)
    {
        table.base_offset = 0;
    }
    else
    {
        table.base_offset = top_symbol()->offset + 4;
    }
}

size_t top_scope()
{
    if (scopes.count == 0)
    {
        return 0;
    }
    return scopes.items[scopes.count - 1];
}

bool contains_key(const char *key)
{
    for (size_t i = 0; i < table.count; ++i)
    {
        if (strcmp(table.items[i].key, key) == 0)
        {
            return true;
        }
    }
    return false;
}

bool contains_in_current_scope(const char *key)
{
    for (size_t i = top_scope(); i < table.count; ++i)
    {
        if (strcmp(table.items[i].key, key) == 0)
        {
            return true;
        }
    }
    return false;
}

SymbolEntry *get_entry(const char *key)
{
    for (ssize_t i = (ssize_t)table.count - 1; i >= 0; i -= 1)
    {
        if (strcmp(table.items[i].key, key) == 0)
        {
            return &table.items[i];
        }
    }
    return NULL;
}

enum ParamResult check_param(Type type)
{
    SymbolEntry *function = param_stack.items[param_stack.count - 1].function;
    size_t *count = &param_stack.items[param_stack.count - 1].count;

    if (*count == function->num_args)
    {
        return Overflow;
    }

    if (function->args[*count] != type)
    {
        return NoMatch;
    }

    *count += 1;
    return Match;
}
