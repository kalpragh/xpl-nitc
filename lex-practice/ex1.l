%{
    #include<stdio.h>
    int charcnt=0, word=0, line=0;
%}
%%
\n {line++; charcnt++;}
[^ \t\n]+ {word++; charcnt+=yyleng;}
. {charcnt++;}
%%
int yywrap(){ return 1; }
int main(){
    yylex();
    printf("charcnt= %d \nwords=%d\nline=%d\n",charcnt,word,line);
    return 0;
}