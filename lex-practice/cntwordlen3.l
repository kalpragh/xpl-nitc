%{
#include<stdio.h>
#include<ctype.h>
int cnt=0;
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

int main(){
    yylex();
    return 0;
}