#include "helpers.h"

#include <stdlib.h>

void generate_code_binary_operation(char *mnemonic, AsdTree *root, struct AsdTree *left, struct AsdTree *right)
{
    root->location = generate_temporary();
    root->code = concatenate_multiple_codes(
        left->code, right->code, generate_code(mnemonic, left->location, right->location, root->location), NULL);
}
