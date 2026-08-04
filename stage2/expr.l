%{
#include"y.tab.h"
#include<stdlib.h>
%}

%%
[0-9]+ {yylval.num=atoi(yytext);return NUM;}
[a-z]  {yylval.id=yytext[0];return ID;}

"begin" {return START;}
"end"   {return END;}
"read"  {return READ;}
"write" {return WRITE;}

[ \t\n] ;
. {return yytext[0];}
%%