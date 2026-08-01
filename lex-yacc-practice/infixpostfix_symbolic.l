%{
#include "y.tab.h"
#include <string.h>
#include <stdlib.h>
%}

%%

[a-zA-Z][a-zA-Z0-9]* {
    yylval.str = strdup(yytext);
    return ID;
}

"+"     return '+';
"-"     return '-';
"*"     return '*';
"/"     return '/';
"("     return '(';
")"     return ')';

[ \t]   ;

\n      return '\n';

.       ;

%%

int yywrap()
{
    return 1;
}