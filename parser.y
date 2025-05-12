%code requires {
    #include "asd.h"
}

%{

#include <stdio.h>
#include <string.h>
#include "asd.h"

int yylex(void);
void yyerror (char const *mensagem);
int get_line_number(void);

extern AsdTree *arvore;

%}

%union {
    AsdTree *node;
    LexicalValue *lexical_value;
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
%type<node> command_block
%type<node> simple_command
%type<node> expression_list
%type<node> expression
%type<node> decl_var_with_initialization
%type<node> attribution
%type<node> func_call
%type<node> return
%type<node> conditional
%type<node> while
%type<node> else
%type<node> literals
%type<node> params_list_
%type<node> params_list
%type<node> param
%type<node> types

%define parse.error verbose

%%

program:
    list ';' {$$=$1; arvore=$$; } | 
    /* %empty*/  {$$ = NULL; arvore=$$;};

list:
    element { $$ = $1; } |
    element ',' list { 
        $$ = $1;
        if($$){
            if($3)
                asd_add_child($$, $3); 
        }else{
            if($3){
                $$ = $3;
            }else{
                $$ = NULL;
            }
        }
    };

element:
    func_def {$$ = $1;} |
    decl_var {$$ = $1;};

types:
    TK_PR_FLOAT {$$ = NULL;} | 
    TK_PR_INT {$$ = NULL;};

param: TK_ID TK_PR_AS types {
        $$ = NULL; 
        free($1->lexem);
        free($1);
        free($3);
    };

params_list_:
    param {$$ = NULL; } |
    params_list_ ',' param  {$$ = NULL; };

params_list: TK_PR_WITH params_list_ {$$ = NULL; };

func_def:
    TK_ID TK_PR_RETURNS types TK_PR_IS command_block {
            $$ = asd_new($1->lexem);
            free($1->lexem); free($1); 
            if($5) {
                asd_add_child($$, $5);
            }
            free($3); 
        } |
    TK_ID TK_PR_RETURNS types params_list TK_PR_IS command_block {
            $$ = asd_new($1->lexem); 
            free($1->lexem); free($1); 
            if($6) {
                asd_add_child($$, $6);
            }
            free($3);
            free($4); 
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
            $$ = $1;
            if($$){
                if($2){
                    asd_add_child($$, $2);
                }
            }else{
                if($2){
                    $$ = $2;
                }else{
                    $$ = NULL;
                }
            }
        };

command_block:
    '[' ']' { $$ = NULL; } |
    '[' command_list ']' { $$ = $2; };

literals: 
    TK_LI_FLOAT { $$ = asd_new($1->lexem); free($1->lexem); free($1); } |
    TK_LI_INT { $$ = asd_new($1->lexem); free($1->lexem); free($1); };


decl_var_with_initialization:
    TK_PR_DECLARE TK_ID TK_PR_AS types TK_PR_WITH literals {$$ = asd_new("with"); asd_add_child($$, asd_new($2->lexem)); asd_add_child($$, $6); free($2->lexem); free($2); } ;

decl_var:
    TK_PR_DECLARE TK_ID TK_PR_AS types {$$=NULL; free($2->lexem); free($2);};

attribution:
    TK_ID TK_PR_IS expression {
        $$ = asd_new("is"); 
        asd_add_child($$, asd_new($1->lexem)); 
        asd_add_child($$, $3); 
        free($1->lexem); free($1);
    }; 

func_call:
    TK_ID '(' expression_list ')'{
        char* combined = malloc(strlen($1->lexem) + 6);
        strcpy(combined, "call ");
        strcat(combined, $1->lexem);
        $$ = asd_new(combined); 
         
        free($1->lexem); free($1);
        free(combined);
        if($3) {
            asd_add_child($$, $3);
        }
    }; 

expression_list:
    /* empty */ {$$ = NULL; } |
    expression {$$ = $1; } |
    expression ',' expression_list {$$ = $1; asd_add_child($$, $3); } ;

return:
    TK_PR_RETURN expression TK_PR_AS types {
        $$ = asd_new("return"); 
        asd_add_child($$, $2);
        free($4);
    } 

conditional:
    TK_PR_IF '(' expression ')' command_block else {
        $$ = asd_new("if");
        if($3){
            asd_add_child($$, $3);
        }
        if($5){
            asd_add_child($$, $5);
        }
        if ($6){
            asd_add_child($$, $6);
        }
    };


else:
    /* empty */ {$$ = NULL;} |
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
        if($3) {
            asd_add_child($$, $3);
        }
        if($5) {
            asd_add_child($$, $5);
        }
    };

expression: 
    n7  {$$ = $1;};

n7:
    n7 '|' n6 { $$ = asd_new("|"); asd_add_child($$, $1);  asd_add_child($$, $3);}|
    n6 { $$ = $1; };

n6:
    n6 '&' n5 { $$ = asd_new("&"); asd_add_child($$, $1);  asd_add_child($$, $3);}|
    n5 { $$ = $1; };

n5:
    n5 TK_OC_EQ n4 { $$ = asd_new("=="); asd_add_child($$, $1);  asd_add_child($$, $3);}|
    n5 TK_OC_NE n4 { $$ = asd_new("!="); asd_add_child($$, $1);  asd_add_child($$, $3);}|
    n4 { $$ = $1; };

n4:
    n4 '<' n3 { $$ = asd_new("<"); asd_add_child($$, $1);  asd_add_child($$, $3);} |
    n4 '>' n3 { $$ = asd_new(">"); asd_add_child($$, $1);  asd_add_child($$, $3);}|
    n4 TK_OC_LE n3 { $$ = asd_new("<="); asd_add_child($$, $1);  asd_add_child($$, $3);}|
    n4 TK_OC_GE n3 { $$ = asd_new(">="); asd_add_child($$, $1);  asd_add_child($$, $3);}|
    n3 { $$ = $1; };

n3:
    n3 '+' n2 { $$ = asd_new("+"); asd_add_child($$, $1);  asd_add_child($$, $3);}|
    n3 '-' n2 { $$ = asd_new("-"); asd_add_child($$, $1);  asd_add_child($$, $3);}|
    n2 { $$ = $1; };

n2:
    n2 '*' n1 { $$ = asd_new("*"); asd_add_child($$, $1);  asd_add_child($$, $3);} |
    n2 '/' n1 { $$ = asd_new("/"); asd_add_child($$, $1);  asd_add_child($$, $3);} |
    n2 '%' n1 { $$ = asd_new("%"); asd_add_child($$, $1);  asd_add_child($$, $3);} |
    n1 { $$ = $1; };

n1:
    '+' n1 { $$ = asd_new("+"); asd_add_child($$, $2); } |
    '-' n1 { $$ = asd_new("-"); asd_add_child($$, $2); } |
    '!' n1 { $$ = asd_new("!"); asd_add_child($$, $2); } |
    n0 { $$ = $1; };

n0: 
    func_call { $$ = $1; } |
    TK_ID { $$ = asd_new($1->lexem); free($1->lexem); free($1); } |
    TK_LI_FLOAT { $$ = asd_new($1->lexem); free($1->lexem); free($1); } | 
    TK_LI_INT { $$ = asd_new($1->lexem); free($1->lexem); free($1); } | 
    '(' expression ')' { if($2){$$ = $2;}; }; 
%%

void yyerror(const char* mensagem)
{
    printf("Na linha %d, houve o erro \"%s\"\n", get_line_number(), mensagem);
}