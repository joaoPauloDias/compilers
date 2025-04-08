%{
#include <stdio.h>
#include "parser.tab.h"
%}

%option noyywrap
%option yylineno

%%
as {return TK_PR_AS;}
declare {return TK_PR_DECLARE;}
else {return TK_PR_ELSE;}
float {return TK_PR_FLOAT;}
if {return TK_PR_IF;}
int {return TK_PR_INT;}
is {return TK_PR_IS;}
return {return TK_PR_RETURN;}
returns {return TK_PR_RETURNS;}
while {return TK_PR_WHILE;}
with {return TK_PR_WITH;}

\<\= return TK_OC_LE;
\>\= return TK_OC_GE;
\=\= return TK_OC_EQ;
\!\= return TK_OC_NE;

\! return '!';
\* return '*';
\/ return '/';
\% return '%';
\+ return '+';
\- return '-';
\< return '<';
\> return '>';
\[ return '[';    
\] return ']';
\( return '(';
\) return ')';
\= return '=';
\, return ',';
\; return ';';
\& return '&';
\| return '|';

[a-z_][a-z_0-9]* {return TK_ID;}
[0-9]+\.[0-9]+ {return TK_LI_FLOAT;}
[0-9]+ {return TK_LI_INT;}

\/\/.*

[\n\t\r ] ;

. return TK_ER;
%%

int get_line_number(void){
    return yylineno;
}