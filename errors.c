#include "errors.h"
#include "types.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

void err_undeclared(const char *symbol, uint32_t line)
{
    fprintf(stderr, "ERROR: The symbol \"%s\" from line %" PRIu32 " was not declared\n", symbol, line);
    exit(ERR_UNDECLARED);
}

void err_declared(const char *symbol, uint32_t line)
{
    fprintf(stderr, "ERROR: The symbol \"%s\" from line %" PRIu32 " was already used\n", symbol, line);
    exit(ERR_DECLARED);
}

void err_variable(const char *symbol, uint32_t line)
{
    fprintf(stderr, "ERROR: The symbol \"%s\" from line %" PRIu32 " is not a function\n", symbol, line);
    exit(ERR_VARIABLE);
}

void err_function(const char *symbol, uint32_t line)
{
    fprintf(stderr, "ERROR: The symbol \"%s\" from line %" PRIu32 " is a function\n", symbol, line);
    exit(ERR_FUNCTION);
}

void err_wrong_type(const char *symbol, uint32_t line)
{
    fprintf(stderr, "ERROR: The symbol \"%s\" from line %" PRIu32 " does not match the type\n", symbol, line);
    exit(ERR_WRONG_TYPE);
}

void err_missing_args()
{
    fprintf(stderr, "ERROR: There are missing args in this function call\n");
    exit(ERR_MISSING_ARGS);
}

void err_excess_args()
{
    fprintf(stderr, "ERROR: This function call has too many args\n");
    exit(ERR_EXCESS_ARGS);
}

void err_wrong_type_args(Type type)
{
    fprintf(stderr, "ERROR: The type \"%s\" does not match the expected function argument type\n", TYPE_LOOKUP[type]);
    exit(ERR_WRONG_TYPE_ARGS);
}
