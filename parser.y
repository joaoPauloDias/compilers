%{

#include <stdio.h>
#include "asd.h"

int yylex(void);
void yyerror (char const *mensagem);
int get_line_number(void);

extern asd_tree_t *arvore;

typedef struct {
    char* lexem;
    size_t line_number;
} value_t;

%}

%union {
    asd_tree_t *node;
    value_t *lexical_value;
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
%type<node> n0
%type<node> n1
%type<node> n2
%type<node> n3
%type<node> n4
%type<node> n5
%type<node> n6
%type<node> n7
%type<node> command_list
%type<node> command_block

%define parse.error verbose

%%

program: /* empty */ | list ';' ;

list:
    element |
    list ',' element;

element: func_def | decl_var;

types: TK_PR_FLOAT | TK_PR_INT;

param: TK_ID TK_PR_AS types;

params_list_:
    param |
    params_list_ ',' param;

params_list: TK_PR_WITH params_list_;

func_def:
    TK_ID TK_PR_RETURNS types TK_PR_IS command_block |
    TK_ID TK_PR_RETURNS types params_list TK_PR_IS command_block;

simple_command: command_block | decl_var | decl_var_with_initialization | attribution | func_call | return | conditional | while;

command_list:
    simple_command { $$ = $1; } |
    simple_command command_list { $$ = $1; asd_add_child($1, $2); };

command_block:
    '[' ']' { $$ = NULL; } |
    '[' command_list ']' { $$ = $2; };

literals: TK_LI_FLOAT | TK_LI_INT;

decl_var: TK_PR_DECLARE TK_ID TK_PR_AS types;

decl_var_with_initialization: TK_PR_DECLARE TK_ID TK_PR_AS types TK_PR_WITH literals;

attribution: TK_ID TK_PR_IS expression;

func_call:
    TK_ID '(' expression_list ')';

expression_list:
    /* empty */ |
    expression |
    expression_list ',' expression;

return: TK_PR_RETURN expression TK_PR_AS types;

conditional: TK_PR_IF '(' expression ')' command_block else;

else:
    /* empty */ |
    TK_PR_ELSE command_block;

while: TK_PR_WHILE '(' expression ')' command_block;

expression: n7;

n7:
    n7 '|' n6 |
    n6 { $$ = $1 };

n6:
    n6 '&' n5 |
    n5 { $$ = $1 };

n5:
    n5 TK_OC_EQ n4 |
    n5 TK_OC_NE n4 |
    n4 { $$ = $1 };

n4:
    n4 '<' n3 |
    n4 '>' n3 |
    n4 TK_OC_LE n3 |
    n4 TK_OC_GE n3 |
    n3 { $$ = $1 };

n3:
    n3 '+' n2 |
    n3 '-' n2 |
    n2 { $$ = $1 };

n2:
    n2 '*' n1 |
    n2 '/' n1 |
    n2 '%' n1 { $$ = asd_new($2); asd_add_child($$, $1); asd_add_child($$, $3); } |
    n1 { $$ = $1 };

n1:
    '+' n1 |
    '-' n1 |
    '!' n1 { $$ = asd_new("!"); asd_add_child($$, $2); } |
    n0 { $$ = $1 };

n0: func_call |
    TK_ID { $$ = asd_new($1->lexem); free($1->lexem); free($1); } |
    TK_LI_FLOAT | 
    TK_LI_INT | 
    '(' expression ')';

%%

void yyerror(const char* mensagem)
{
    printf("Na linha %d, houve o erro \"%s\"\n", get_line_number(), mensagem);
}