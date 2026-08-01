%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int yylex();
void yyerror(char *s);

/* Function to create prefix expression */
char *makePrefix(char *op, char *left, char *right)
{
    char *result = (char *)malloc(strlen(op) + strlen(left) + strlen(right) + 5);
    sprintf(result, "%s %s %s", op, left, right);
    return result;
}
%}

%union{
    char *str;
}

%token <str> ID
%type <str> E T F

%%

S : E '\n'
    {
        printf("%s\n", $1);
    }
  ;

E : E '+' T
    {
        $$ = makePrefix("+", $1, $3);
    }
  | E '-' T
    {
        $$ = makePrefix("-", $1, $3);
    }
  | T
    {
        $$ = $1;
    }
  ;

T : T '*' F
    {
        $$ = makePrefix("*", $1, $3);
    }
  | T '/' F
    {
        $$ = makePrefix("/", $1, $3);
    }
  | F
    {
        $$ = $1;
    }
  ;

F : '(' E ')'
    {
        $$ = $2;
    }
  | ID
    {
        $$ = $1;
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