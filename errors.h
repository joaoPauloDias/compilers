#include "types.h"

#include <stdint.h>

#define ERR_UNDECLARED 10      // 2.2
#define ERR_DECLARED 11        // 2.2
#define ERR_VARIABLE 20        // 2.3
#define ERR_FUNCTION 21        // 2.3
#define ERR_WRONG_TYPE 30      // 2.4
#define ERR_MISSING_ARGS 40    // 2.5
#define ERR_EXCESS_ARGS 41     // 2.5
#define ERR_WRONG_TYPE_ARGS 42 // 2.5

void err_undeclared(const char *symbol, uint32_t line);

void err_declared(const char *symbol, uint32_t line);

void err_variable(const char *symbol, uint32_t line);

void err_function(const char *symbol, uint32_t line);

void err_wrong_type(const char *symbol, uint32_t line);

void err_missing_args();

void err_excess_args();

void err_wrong_type_args(Type type);
