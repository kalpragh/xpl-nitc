%{
#include "y.tab.h"
%}

%%

[a-zA-Z] {
    yylval.c = yytext[0];
    return ID;
}

"+" return '+';
"-" return '-';
"*" return '*';
"/" return '/';
"(" return '(';
")" return ')';

\n return '\n';

[ \t] ;

. ;

%%

int yywrap()
{
    return 1;
}