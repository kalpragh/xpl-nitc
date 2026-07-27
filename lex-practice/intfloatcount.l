%{
#include<stdio.h>
int intcnt=0, floatcnt=0;
%}

%%
[0-9]+\.[0-9]+ {floatcnt++;}
[0-9]+ {intcnt++;}
.|\n ;
%%
int main(){
    yylex();
    printf("integers: %d\nfloats: %d\n",intcnt,floatcnt);
    return 0;
}