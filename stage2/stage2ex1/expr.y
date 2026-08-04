%{
#include<stdio.h>
#include<stdlib.h>
#include<string.h>   // ✅ IMPORTANT
#include"tnode.h"

struct tnode *root;

int yylex(void);
int yyerror(char *s);
extern FILE *yyin;
int memory[26] = {0};   // ✅ initialize
%}

%union{
    struct tnode *node;
}

%token <node> NUM ID
%token START END READ WRITE
%left '+'
%left '*'

%type <node> Program Slist Stmt Expr

%%

Program:
    START Slist END ';' { root = $2; }
;

Slist:
    Slist Stmt { $$ = createtree(0,'C',NULL,$1,$2); }
  | Stmt       { $$ = $1; }
;

Stmt:
    ID '=' Expr ';'
    {
        $$ = createtree(0,'=',NULL,$1,$3);
    }

  | READ '(' ID ')' ';'
    {
        $$ = createtree(0,'R',NULL,$3,NULL);
    }

  | WRITE '(' Expr ')' ';'
    {
        $$ = createtree(0,'W',NULL,$3,NULL);
    }
;


Expr:
      Expr '+' Expr { $$ = createtree(0,'+',NULL,$1,$3); }
    | Expr '*' Expr { $$ = createtree(0,'*',NULL,$1,$3); }
    | NUM           { $$ = $1; }  
    | ID            { $$ = $1; }   
;

%%

int evaluate(struct tnode *t){

    if(t->nodetype == 'N')
        return t->val;

    if(t->nodetype == 'V')
        return memory[t->varname[0] - 'a'];

    if(t->nodetype == '+')
        return evaluate(t->left) + evaluate(t->right);

    if(t->nodetype == '*')
        return evaluate(t->left) * evaluate(t->right);

    if(t->nodetype == '=')
    {
        int val = evaluate(t->right);
        memory[t->left->varname[0] - 'a'] = val;
        return val;
    }

    if(t->nodetype == 'R')
    {
        int x;
        scanf("%d", &x);
        memory[t->left->varname[0] - 'a'] = x;
        return 0;
    }

    if(t->nodetype == 'W')
    {
        int val = evaluate(t->left);
        printf("%d\n", val);
        return 0;
    }

    if(t->nodetype == 'C')
    {
        evaluate(t->left);
        evaluate(t->right);
        return 0;
    }

    return 0;
}

int yyerror(char *s){
    printf("Error: %s\n", s);
    return 1;
}
int main(){
    yyin = fopen("input.txt", "r");
    if (yyin == NULL) {
        printf("Cannot open input file\n");
        return 1;
    }

    yyparse();
    fclose(yyin);

    printf("\n");
    evaluate(root);
    return 0;
}