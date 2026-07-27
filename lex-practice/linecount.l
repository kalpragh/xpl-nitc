%{
#include<stdio.h>
int lines=0, words=0, chars=0;
%}
%%
\n {lines++; chars++;}
[^ \t\n]+ {words++;chars+=yyleng;}
.  {chars++;}
%%
int main(){
    yylex();
    printf("lines: %d\nWords: %d\nCharacters: %d\n",lines,words,chars);
    return 0;
}