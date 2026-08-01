%{
#include<stdio.h>
#include<stdlib.h>

void yyerror(char *s);
int yylex();
%}

%union{
    char c;
}

%token <c> ID

%%

S : E '\n'
  ;

E : E '+' T   { printf("+"); }
  | E '-' T   { printf("-"); }
  | T
  ;

T : T '*' F   { printf("*"); }
  | T '/' F   { printf("/"); }
  | F
  ;

F : '(' E ')'
  | ID         { printf("%c", $1); }
  ;

%%

int main()
{
    yyparse();
    return 0;
}

void yyerror(char *s)
{
    printf("Error\n");
}