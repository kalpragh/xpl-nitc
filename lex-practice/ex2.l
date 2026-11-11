%{
#include<stdio.h>
int incnt=0,floatcnt;
%}
%%
[0-9]+\.[0-9]+ {floatcnt++;}
[0-9]+ {incnt++;}
.|\n ;
%%
int yywrap(){
    return 1;
}
int main(){
    yylex();
    printf("int count=%d \nfloatcnt=%d\n",incnt,floatcnt);
    return 0;
}