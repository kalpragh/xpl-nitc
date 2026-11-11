%{
#include<stdio.h>
#include<ctype.h>
%}
%%
[Aa][a-zA-Z]{2} {
    int i;
    for(i=0;yytext[i];i++)
        putchar(toupper(yytext[i]));
    putchar('\n');
}
[a-zA-Z]+ ;
.|\n ;
%%
int yywrap(){
    return 1;
}
int main(){
    yylex();
    return 0;
}