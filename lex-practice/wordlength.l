%{
#include <stdio.h>
%}

%%
[0-9]+ {
    
printf("length = %d\n", yyleng);
}
%%
int yywrap() {
return 1;
}

int main() {
yylex();
return 0;
}