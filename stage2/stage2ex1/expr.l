%{
#include "y.tab.h"
#include "tnode.h"
#include <stdlib.h>
#include <string.h>
%}

%%
[0-9]+ {
    yylval.node = createtree(atoi(yytext), 'N', NULL, NULL, NULL);
    return NUM;
}

[a-z] {
    yylval.node = createtree(0, 'V', strdup(yytext), NULL, NULL);
    return ID;
}

"begin" { return START; }
"end"   { return END; }
"read"  { return READ; }
"write" { return WRITE; }

[ \t\n] ;

. { return yytext[0]; }
%%