%code requires {
    #include "asd.h"
}

%{

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "types.h"
#include "errors.h"
#include "asd.h"
#include "arena.h"
#include "symbol_table.h"
#include "code_utils.h"

int yylex(void);
void yyerror (char const *mensagem);
int get_line_number(void);

extern AsdTree *arvore;

const int CODE_OFFSET_SIZE = 16;

SymbolEntry* current_function;

%}

%union {
    AsdTree *node;
    LexicalValue *lexical_value;
    Type type;
}

%token TK_PR_AS
%token TK_PR_DECLARE
%token TK_PR_ELSE
%token TK_PR_FLOAT
%token TK_PR_IF
%token TK_PR_INT
%token TK_PR_IS
%token TK_PR_RETURN
%token TK_PR_RETURNS
%token TK_PR_WHILE
%token TK_PR_WITH
%token TK_OC_LE
%token TK_OC_GE
%token TK_OC_EQ
%token TK_OC_NE
%token<lexical_value> TK_ID 
%token<lexical_value> TK_LI_INT
%token<lexical_value> TK_LI_FLOAT
%token TK_ER

%type<node> program
%type<node> list
%type<node> element
%type<node> func_def
%type<node> decl_var
%type<node> n0
%type<node> n1
%type<node> n2
%type<node> n3
%type<node> n4
%type<node> n5
%type<node> n6
%type<node> n7
%type<node> command_list
%type<node> function_block
%type<node> command_block
%type<node> simple_command
%type<node> params
%type<node> expression
%type<node> decl_var_with_initialization
%type<node> attribution
%type<node> func_call
%type<node> return
%type<node> conditional
%type<node> while
%type<node> else
%type<node> literal
%type<node> params_list
%type<node> param
%type<type> type

%define parse.error verbose

%%

program:
    list ';' { $$=$1; arvore=$$;} |
    /* %empty*/  { $$ = NULL; arvore=$$; };

list:
    element { $$ = $1; } |
    element ',' list { 
        $$ = $1;
        if($$) {
            if($3) {
                asd_add_child($$, $3);
                $$->location = generate_temporary();
                $$->code = concatenate_code($1->code, $3->code);
            }
        } else {
            if($3) {
                $$ = $3;
            } else {
                $$ = NULL;
            }
        }
    };

element:
    func_def {$$ = $1;} |
    decl_var {$$ = $1;};

type:
    TK_PR_FLOAT {
        $$ = TFloat;
    } |
    TK_PR_INT {
        $$ = TInt;
    };

param: TK_ID TK_PR_AS type {
    $$ = NULL;

    if (contains_in_current_scope($1->lexem)) {
        err_declared($1->lexem, get_line_number());
    }

    push_symbol((SymbolEntry) {.key=arena_strdup(allocator, $1->lexem), .type=$3, .nature=SyIdentifier});

    free($1->lexem);
    free($1);
};

params_list:
    param { $$ = NULL; } |
    params_list ',' param { $$ = NULL; };

func_def:
    TK_ID TK_PR_RETURNS type {
        if (contains_key($1->lexem)) {
            err_declared($1->lexem, get_line_number());
        }

        push_symbol((SymbolEntry) {.key=$1->lexem, .type=$3, .nature=SyFunction});
        current_function = top_symbol();
    }
    TK_PR_IS command_block {
        $$ = asd_new_ownership($1->lexem);

        free($1);
        if($6) {
            asd_add_child($$, $6);
            $$->code = $6->code;
        }
    } |
    TK_ID TK_PR_RETURNS type {
        if (contains_key($1->lexem)) {
            err_declared($1->lexem, get_line_number());
        }
        
        push_symbol((SymbolEntry) {.key=$1->lexem, .type=$3, .nature=SyFunction});
        current_function = top_symbol();

        push_scope();
    }
    TK_PR_WITH params_list {
        size_t current_scope = scopes.items[scopes.count - 1];
        size_t num_args = table.count - current_scope;
        Type* args = arena_alloc(allocator, num_args * sizeof(Type));

        size_t j = 0;
        for (size_t i = current_scope; i < table.count; ++i) {
            args[j] = table.items[i].type;
            j += 1;
        };

        current_function->num_args = num_args;
        current_function->args = args;
    }
    TK_PR_IS function_block {
        pop_scope();

        $$ = asd_new_ownership($1->lexem);

        if ($9) {
            asd_add_child($$, $9);
            $$->code = $9->code; 
        }

        free($1);
    };
    
simple_command:
    command_block {$$ = $1;} | 
    decl_var {$$ = $1;} |
    decl_var_with_initialization {$$ = $1;} |
    attribution {$$ = $1;} |
    func_call {$$ = $1;} | 
    return {$$ = $1;} | 
    conditional {$$ = $1;} | 
    while {$$ = $1;};

command_list:
    simple_command { $$ = $1; } |
    simple_command command_list {
        if($1) {
            $$ = $1;
            if($2) {
                asd_add_child($$, $2);
                $$->location = generate_temporary();
                $$->code = concatenate_code($1->code, $2->code);
            }
        } else {
            if($2) {
                $$ = $2;
            } else {
                $$ = NULL;
            }
        }
    };

function_block:
    '[' ']' { $$ = NULL; } |
    '[' command_list ']' {  $$ = $2; };

command_block:
    '[' ']' { $$ = NULL; } |
    '[' { push_scope(); } command_list ']' { pop_scope(); $$ = $3;};

literal: 
    TK_LI_FLOAT { 
        $$ = asd_new_ownership($1->lexem);
        $$->type = TFloat;
        free($1);
        $$->code = NULL;
    } |
    TK_LI_INT {
        $$ = asd_new_ownership($1->lexem);
        $$->type = TInt;
        free($1);
        $$->location = generate_temporary();
        $$->code = generate_code("loadI", $$->label, NULL, $$->location);
    };

decl_var_with_initialization:
    TK_PR_DECLARE TK_ID TK_PR_AS type TK_PR_WITH literal {
        if (contains_in_current_scope($2->lexem)) {
            err_declared($2->lexem, get_line_number());
        }

        if ($6->type != $4) {
            err_wrong_type($6->label, get_line_number());
        }

        $$ = asd_new("with");
        asd_add_child($$, asd_new_ownership($2->lexem));
        $$->type = $4;

        push_symbol((SymbolEntry) {.key=$2->lexem, .nature=SyIdentifier, .type=$4});

         
        char offset_str[CODE_OFFSET_SIZE];
        sprintf(offset_str, "%zu", (size_t)top_symbol()->offset);

        $$->code = concatenate_code(
            $6->code,
            generate_code("storeAI", $6->location, get_entry($2->lexem)->is_global ? "rbss": "rfp", offset_str)
        );

        free($2);

        asd_add_child($$, $6);
    };

decl_var:
    TK_PR_DECLARE TK_ID TK_PR_AS type {
        if (contains_in_current_scope($2->lexem)) {
            err_declared($2->lexem, get_line_number());
        }

        $$ = NULL;

        push_symbol((SymbolEntry) {.key=arena_strdup(allocator, $2->lexem), .nature=SyIdentifier, .type=$4});

        free($2->lexem);
        free($2);
    };

attribution:
    TK_ID TK_PR_IS expression {
        SymbolEntry* entry = get_entry($1->lexem);

        if (entry == NULL) {
            err_undeclared($1->lexem, get_line_number());
        }

        if (entry->nature != SyIdentifier)
        {
            err_function($1->lexem, get_line_number());
        }

        if (entry->type != $3->type)
        {
            err_wrong_type($1->lexem, get_line_number());
        }

        $$ = asd_new("is");
        $$->type = $3->type;

        asd_add_child($$, asd_new_ownership($1->lexem)); 
        asd_add_child($$, $3); 

        char offset_str[CODE_OFFSET_SIZE];
        sprintf(offset_str, "%zu", entry->offset);
        code_t* store_code = generate_code("storeAI", $3->location, get_entry($1->lexem)->is_global ? "rbss": "rfp", offset_str);
        $$->code = concatenate_code($3->code, store_code);

        free($1);
    }; 

func_call:
    TK_ID {
        SymbolEntry* entry = get_entry($1->lexem);

        if (entry == NULL) {
            err_undeclared($1->lexem, get_line_number());
        }

        if (entry->nature != SyFunction) {
            err_variable($1->lexem, get_line_number());
        }

        arena_da_append(allocator, &param_stack, (typeof(*param_stack.items)) {.function = entry});
    }
    '(' params ')' {
        SymbolEntry* entry = get_entry($1->lexem);
        if (param_stack.items[param_stack.count - 1].count < entry->num_args) {
            err_missing_args();
        }

        char* combined = malloc(strlen($1->lexem) + 6);
        strcpy(combined, "call ");
        strcat(combined, $1->lexem);

        $$ = asd_new_ownership(combined);
        $$->type = entry->type;
         
        free($1->lexem); free($1);
        if($4) {
            asd_add_child($$, $4);
        }

        param_stack.count -= 1;
    }; 

params:
    /* empty */ { $$ = NULL; } |
    expression {
        enum ParamResult result = check_param($1->type);
        if (result == NoMatch) {
            err_wrong_type_args($1->type);
        } else if (result == Overflow) {
            err_excess_args();
        }
        $$ = $1;
    } |
    expression ',' params {
        enum ParamResult result = check_param($1->type);
        if (result == NoMatch) {
            err_wrong_type_args($1->type);
        } else if (result == Overflow) {
            err_excess_args();
        }
        $$ = $1;
        asd_add_child($$, $3);
    };

return:
    TK_PR_RETURN expression TK_PR_AS type {
        if (current_function->type != $4) {
            err_wrong_type("return", get_line_number());
        }

        if ($2->type != $4) {
            err_wrong_type("expression", get_line_number());
        }

        $$ = asd_new("return");
        $$->code = $2->code;
        asd_add_child($$, $2);
    }

conditional:
    TK_PR_IF '(' expression ')' command_block else {
        $$ = asd_new("if");
        $$->type = $3->type;

        asd_add_child($$, $3);

        if ($5) {
            asd_add_child($$, $5);
        }

        if ($6) {
            if ($5 != NULL && $5->type != $6->type) {
                err_wrong_type("else", get_line_number());
            }
            asd_add_child($$, $6);
        }

             
        char *label_true = generate_label();
        char *label_false = generate_label();
        char *label_end = generate_label();

        code_t *code_cbr = generate_code("cbr", $3->location, label_true, label_false);
        
        code_t *code_jump = generate_code("jumpI", NULL, label_end, NULL);
        
        code_t *code_nop = generate_code("nop", NULL, NULL, NULL);
        code_nop->instruction.label = label_end;

        code_t *code_true = $5 == NULL ? generate_code("nop", NULL, NULL, NULL) : $5->code;
        code_true->instruction.label = label_true;

        code_t *code_false = $6 == NULL ? generate_code("nop", NULL, NULL, NULL) : $6->code;
        code_false->instruction.label = label_false;


        $$->code = concatenate_multiple_codes(
            $3->code,
            code_cbr,
            code_true,
            code_jump,
            code_false,
            code_nop,
            NULL
        );
    };


else:
    /* empty */ { $$ = NULL; } |
    TK_PR_ELSE command_block {
        if ($2) {
            $$ = $2;
        } else {
            $$ = NULL;
        }
    };

while:
    TK_PR_WHILE '(' expression ')' command_block {
        $$ = asd_new("while");

        $$->type = $3->type;
        asd_add_child($$, $3);

        if($5) {
            asd_add_child($$, $5);
        }

        char *label_true = generate_label();
        char *label_false = generate_label();
        char *label_back = generate_label();

        $3->code->instruction.label = label_back;

        code_t *code_cbr = generate_code("cbr", $3->location, label_true, label_false);
    
        code_t *code_jump = generate_code("jumpI", NULL, label_back, NULL);

        code_t *code_true = $5 == NULL ? generate_code("nop", NULL, NULL, NULL) : $5->code;
        code_true->instruction.label = label_true;

        code_t *code_false = generate_code("nop", NULL, NULL, NULL);
        code_false->instruction.label = label_false;

        $$->code = concatenate_multiple_codes(
            $3->code,
            code_cbr,
            code_true,
            code_jump,
            code_false,
            NULL
        );
    };

expression: 
    n7  {$$ = $1;};

n7:
    n7 '|' n6 {
        $$ = asd_new("|"); asd_add_child($$, $1);  asd_add_child($$, $3);
        if ($1->type == $3->type) {
            $$->type = $1->type;
            generate_code_binary_operation("or", $$, $1, $3); 
        } else {
            err_wrong_type($$->label, get_line_number());
        }
    }|
    n6 { $$ = $1; };

n6:
    n6 '&' n5 {
        $$ = asd_new("&"); asd_add_child($$, $1);  asd_add_child($$, $3);
        if ($1->type == $3->type) {
            $$->type = $1->type;
            generate_code_binary_operation("and", $$, $1, $3);
        } else {
            err_wrong_type($$->label, get_line_number());
        }
    }|
    n5 { $$ = $1; };

n5:
    n5 TK_OC_EQ n4 {
        $$ = asd_new("=="); asd_add_child($$, $1);  asd_add_child($$, $3);
        if ($1->type == $3->type) {
            $$->type = $1->type;
            generate_code_binary_operation("cmp_EQ", $$, $1, $3); 
        } else {
            err_wrong_type($$->label, get_line_number());
        }
    }|
    n5 TK_OC_NE n4 {
        $$ = asd_new("!="); asd_add_child($$, $1);  asd_add_child($$, $3);
        if ($1->type == $3->type) {
            $$->type = $1->type;
            generate_code_binary_operation("cmp_NE", $$, $1, $3);
        } else {
            err_wrong_type($$->label, get_line_number());
        }
    }|
    n4 { $$ = $1; };

n4:
    n4 '<' n3 {
        $$ = asd_new("<"); asd_add_child($$, $1);  asd_add_child($$, $3);
        if ($1->type == $3->type) {
            $$->type = $1->type;
            generate_code_binary_operation("cmp_LT", $$, $1, $3); 
        } else {
            err_wrong_type($1->label, get_line_number());
        }
    } |
    n4 '>' n3 {
        $$ = asd_new(">"); asd_add_child($$, $1);  asd_add_child($$, $3);
        if ($1->type == $3->type) {
            $$->type = $1->type;
            generate_code_binary_operation("cmp_GT", $$, $1, $3);
        } else {
            err_wrong_type($1->label, get_line_number());
        }
    }|
    n4 TK_OC_LE n3 {
        $$ = asd_new("<="); asd_add_child($$, $1);  asd_add_child($$, $3);
        if ($1->type == $3->type) {
            $$->type = $1->type;
            generate_code_binary_operation("cmp_LE", $$, $1, $3);
        } else {
            err_wrong_type($1->label, get_line_number());
        }
    }|
    n4 TK_OC_GE n3 {
        $$ = asd_new(">="); asd_add_child($$, $1);  asd_add_child($$, $3);
        if ($1->type == $3->type) {
            $$->type = $1->type;
            generate_code_binary_operation("cmp_GE", $$, $1, $3);
        } else {
            err_wrong_type($1->label, get_line_number());
        }
    }|
    n3 { $$ = $1; };

n3:
    n3 '+' n2 {
        $$ = asd_new("+"); asd_add_child($$, $1);  asd_add_child($$, $3);
        if ($1->type == $3->type) {
            $$->type = $1->type;
            generate_code_binary_operation("add", $$, $1, $3);
        } else {
            err_wrong_type($1->label, get_line_number());
        }
    }|
    n3 '-' n2 {
        $$ = asd_new("-"); asd_add_child($$, $1);  asd_add_child($$, $3);
        if ($1->type == $3->type) {
            $$->type = $1->type;
            generate_code_binary_operation("sub", $$, $1, $3);
        } else {
            err_wrong_type($1->label, get_line_number());
        }
    }|
    n2 { $$ = $1; };

n2:
    n2 '*' n1 {
        $$ = asd_new("*"); asd_add_child($$, $1);  asd_add_child($$, $3);
        if ($1->type == $3->type) {
            $$->type = $1->type;
            generate_code_binary_operation("mult", $$, $1, $3);
        } else {
            err_wrong_type($1->label, get_line_number());
        }
    } |
    n2 '/' n1 {
        $$ = asd_new("/"); asd_add_child($$, $1);  asd_add_child($$, $3);
        if ($1->type == $3->type) {
            $$->type = $1->type;
            generate_code_binary_operation("div", $$, $1, $3);
        } else {
            err_wrong_type($1->label, get_line_number());
        }
    } |
    n2 '%' n1 {
        $$ = asd_new("%"); asd_add_child($$, $1);  asd_add_child($$, $3);
        if ($1->type == $3->type) {
            $$->type = $1->type;
        } else {
            err_wrong_type($1->label, get_line_number());
        }
    } |
    n1 { $$ = $1; };

n1:
    '+' n1 {
            $$ = asd_new("+");
            asd_add_child($$, $2);
            $$->type = $2->type;  
            $$->location = generate_temporary();
            $$->code = concatenate_code($2->code, generate_code("addI", $2->location, "0", $$->location)); } |
    '-' n1 { 
            $$ = asd_new("-");
            asd_add_child($$, $2);
            $$->type = $2->type;
            $$->location = generate_temporary();
            $$->code = concatenate_code($2->code, generate_code("rsubI", $2->location, "0", $$->location)); } |
    '!' n1 { 
            $$ = asd_new("!"); 
            asd_add_child($$, $2);
            $$->type = $2->type;

            $$->location = generate_temporary();
            char *temp0 = generate_temporary();

            $$->code = concatenate_multiple_codes(
                $2->code,
                generate_code("loadI", "0", NULL, temp0),
                generate_code("cmp_EQ", $2->location, temp0, $$->location),
                NULL
            ); } |
    n0 { $$ = $1; };

n0: 
    func_call { $$ = $1; } |
    TK_ID {
        SymbolEntry* entry = get_entry($1->lexem);
        if (entry == NULL) {
            err_undeclared($1->lexem, get_line_number());
        }

        if (entry->nature == SyFunction) {
            err_function($1->lexem, get_line_number());
        }

        $$ = asd_new_ownership($1->lexem);
        $$->type = get_entry($1->lexem)->type;
        char buf[CODE_OFFSET_SIZE];
        snprintf(buf, sizeof(buf), "%d", (int)get_entry($1->lexem)->offset);
        $$->location = generate_temporary();
        $$->code = generate_code("loadAI", get_entry($1->lexem)->is_global ? "rbss": "rfp", buf, $$->location);

        free($1);
    } |
    TK_LI_FLOAT { $$ = asd_new_ownership($1->lexem); free($1); $$->type = TFloat; } |
    TK_LI_INT { $$ = asd_new_ownership($1->lexem); 
                $$->type = TInt;  
                $$->location = generate_temporary();
                $$->code = generate_code("loadI", $$->label, NULL, $$->location);
                free($1); 
                } |
    '(' expression ')' { if($2) {$$ = $2;}; };
%%

void yyerror(const char* mensagem)
{
    fprintf(stderr, "ERROR: at line %d, \"%s\"\n", get_line_number(), mensagem);
}
