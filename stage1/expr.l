%{
#include "y.tab.h"
#include "expr.h"
#include <stdlib.h>
%}
%%
[0-9]+  {yylval.node = makeleafnode(atoi(yytext)); return NUM;}
"+"     {return '+';}
"("     {return '(';}
")"     {return ')';}
[ \t]   ;
\n      {return '\n';}
.       {return yytext[0];}
%%
int yywrap(){return 1;}