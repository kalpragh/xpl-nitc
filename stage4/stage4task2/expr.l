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

"decl" { return DECL; }
"enddecl" { return ENDDECL; }
"int" { return INT; }
"str" { return STR; }

[a-zA-Z][a-zA-Z0-9]* {
    yylval.node = malloc(sizeof(struct tnode));
    yylval.node->val = 0;
    yylval.node->type = TYPE_NONE;
    yylval.node->nodetype = 'V';
    yylval.node->varname = strdup(yytext);
    yylval.node->Gentry = NULL;
    yylval.node->left = NULL;
    yylval.node->middle = NULL;
    yylval.node->right = NULL;
    return ID;
}


[ \t\n] ;

. { return yytext[0]; }

%%

int yywrap(){
    return 1;
}