%{
#include <stdio.h>
int yylex(void);
void yyerror (char const *mensagem);
int get_line_number(void);
%}

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
%token TK_ID
%token TK_LI_INT
%token TK_LI_FLOAT
%token TK_ER

%define parse.error verbose

%%

programa: lista ';';
programa: ;

lista: elemento;
lista: lista ',' elemento;

elemento: def_func;
elemento: decl_var;

def_func: ;

decl_var: ;

atribuicao: TK_ID TK_PR_IS expressao;

expressao: n7;

n7: n7 '|' n6;
n7: n6;

n6: n6 '&' n5;
n6: n5;

n5: ;

%%


void yyerror(const char* mensagem)
{
    printf("Na linhda %d, houve o erro \"%s\"\n", get_line_number(), mensagem);
}