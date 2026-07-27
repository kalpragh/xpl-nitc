%{
#include<stdio.h>
#include<ctype.h>
%}
%token LETTER DIGIT
%%
start : variable '\n'  { printf("Valid variable\n"); exit(0); }
      ;
variable : LETTER rest
         ;
rest : rest LETTER
     | rest DIGIT
     |
     ;
%%
yyerror(char *s){ printf("Invalid variable\n"); exit(0); }

yylex()
{
    char c = getchar();
    if(isalpha(c)) return LETTER;
    if(isdigit(c)) return DIGIT;
    return c;
}

main(){ yyparse(); return 0; }