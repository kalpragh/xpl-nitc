%{
#include <stdio.h>
#include <stdlib.h>

int yylex();
void yyerror(char *s);
%}

%union{
    char *str;
}

%token <str> ID

%%

S : E '\n'
  ;

E : E '+' T
    {
        printf("+ ");
    }
  | E '-' T
    {
        printf("- ");
    }
  | T
  ;

T : T '*' F
    {
        printf("* ");
    }
  | T '/' F
    {
        printf("/ ");
    }
  | F
  ;

F : '(' E ')'
  | ID
    {
        printf("%s ", $1);
    }
  ;

%%

int main()
{
    printf("Enter expression: ");
    yyparse();
    return 0;
}

void yyerror(char *s)
{
    printf("Invalid Expression\n");
}