%{
#include "y.tab.h"
#include "tnode.h"
#include <stdlib.h>
#include <string.h>
%}
%option yylineno
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

"return" { return RETURN; }
"tuple" { return TUPLE; }


\"[^\"\n]*\" {
    yytext[yyleng-1] = '\0';
    yylval.node = createtree(0, TYPE_STR, NODE_STRCONST, yytext+1, NULL, NULL, NULL);
    return STRING;
}

"type"        { return TYPE; }
"endtype"     { return ENDTYPE; }
"NULL"|"null"  { return NULLT; }
"alloc"       { return ALLOC; }
"free"        { return FREE; }
"initialize"  { return INITIALIZE; }
"class"        {return CLASS; }
"endclass"      {return ENDCLASS; }
"extends"   { return EXTENDS; }
"new"       { return NEW; }
"delete"    { return DELETE; }
"self"      { return SELF; }
[a-zA-Z_][a-zA-Z0-9_]* {
    yylval.node = calloc(1, sizeof(struct tnode));
    yylval.node->nodetype = 'V';
    yylval.node->varname = strdup(yytext);
    return ID;
}


[ \t\n] ;

. { return yytext[0]; }

%%

int yywrap(){
    return 1;
}