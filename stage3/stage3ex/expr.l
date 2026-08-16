%{
#include "y.tab.h"
#include "tnode.h"
#include <stdlib.h>
#include <string.h>
%}

%%

[0-9]+ {
    yylval.node = createtree(atoi(yytext), TYPE_INT, 'N', NULL, NULL, NULL, NULL);
    return NUM;
}

[a-z] {
    yylval.node = createtree(0, TYPE_INT, 'V', strdup(yytext), NULL, NULL, NULL);
    return ID;
}

"if" { return IF; }
"then" { return THEN; }
"else" { return ELSE; }
"endif" { return ENDIF; }

"while" { return WHILE; }
"do" { return DO; }
"endwhile" { return ENDWHILE; }

"break" {return BREAK;}
"continue" {return CONTINUE; }

"repeat" {return REPEAT; }
"until" {return UNTIL; }

"<=" { return LE; }
">=" { return GE; }
"==" { return EQ; }
"!=" { return NE; }
"<" { return '<'; }
">" { return '>'; }

"begin" { return START; }
"end"   { return END; }
"read"  { return READ; }
"write" { return WRITE; }

[ \t\n] ;

. { return yytext[0]; }

%%

int yywrap(){
    return 1;
}