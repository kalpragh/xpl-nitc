%{
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"tnode.h"

struct tnode *root;

int codegen(struct tnode *t);
int yylex(void);
int yyerror(char *s);
extern FILE *yyin;
FILE *fp;

int memory[26] = {0};
%}

%union{
    struct tnode *node;
}

%token <node> NUM ID
%token START END READ WRITE
%token IF THEN ELSE ENDIF
%token WHILE DO ENDWHILE
%token LE GE EQ NE


%left '<' '>' LE GE EQ NE
%left '+' '-'
%left '*' '/'

%type <node> Program Slist Stmt Expr

%%

Program:
    START Slist END ';' { root = $2; }
;

Slist:
    Slist Stmt { $$ = createtree(0,TYPE_NONE,'C',NULL,$1,NULL,$2); }
  | Stmt       { $$ = $1; }
;

Stmt:
    ID '=' Expr ';'
    {
        $$ = createtree(0,TYPE_NONE,'=',NULL,$1,NULL,$3);
    }

  | READ '(' ID ')' ';'
    {
        $$ = createtree(0,TYPE_NONE,'R',NULL,$3,NULL,NULL);
    }

  | WRITE '(' Expr ')' ';'
    {
        $$ = createtree(0,TYPE_NONE,'W',NULL,$3,NULL,NULL);
    }

  | IF '(' Expr ')' THEN Slist ELSE Slist ENDIF ';'
    {
        $$ = createtree(0,TYPE_NONE,'I',NULL,$3,$6,$8);
    }

  | IF '(' Expr ')' THEN Slist ENDIF ';'
    {
        $$ = createtree(0,TYPE_NONE,'I',NULL,$3,$6,NULL);
    }

  | WHILE '(' Expr ')' DO Slist ENDWHILE ';'
    {
        $$ = createtree(0,TYPE_NONE,'L',NULL,$3,$6,NULL);
    }
;

Expr:
    Expr '+' Expr { $$ = createtree(0,TYPE_NONE,'+',NULL,$1,NULL,$3); }
  | Expr '-' Expr { $$ = createtree(0,TYPE_NONE,'-',NULL,$1,NULL,$3); }
  | Expr '*' Expr { $$ = createtree(0,TYPE_NONE,'*',NULL,$1,NULL,$3); }
  | Expr '/' Expr { $$ = createtree(0,TYPE_NONE,'/',NULL,$1,NULL,$3); }
  | Expr '<' Expr { $$ = createtree(0,TYPE_NONE,'<',NULL,$1,NULL,$3); }
  | Expr '>' Expr { $$ = createtree(0,TYPE_NONE,'>',NULL,$1,NULL,$3); }
  | Expr LE Expr { $$ = createtree(0,TYPE_NONE,NODE_LE,NULL,$1,NULL,$3); }
  | Expr GE Expr { $$ = createtree(0,TYPE_NONE,NODE_GE,NULL,$1,NULL,$3); }
  | Expr EQ Expr { $$ = createtree(0,TYPE_NONE,NODE_EQ,NULL,$1,NULL,$3); }
  | Expr NE Expr { $$ = createtree(0,TYPE_NONE,NODE_NE,NULL,$1,NULL,$3); }
  | '(' Expr ')'  { $$ = $2; }
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

    if(t->nodetype == '-')
        return evaluate(t->left) - evaluate(t->right);

    if(t->nodetype == '*')
        return evaluate(t->left) * evaluate(t->right);

    if(t->nodetype == '/')
        return evaluate(t->left) / evaluate(t->right);

    if(t->nodetype == '<')
        return evaluate(t->left) < evaluate(t->right);

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
        printf("%d\n", evaluate(t->left));
        return 0;
    }

    if(t->nodetype == 'C')
    {
        evaluate(t->left);
        evaluate(t->right);
        return 0;
    }

    if(t->nodetype == 'I'){
        if(evaluate(t->left))
            evaluate(t->middle);
        else if(t->right != NULL)
            evaluate(t->right);
        return 0;
    }

    if(t->nodetype == 'L'){
        while(evaluate(t->left))
            evaluate(t->middle);
        return 0;
    }

    return 0;
}
void printtree(struct tnode *t){
    if(t==NULL)return;
    switch(t->nodetype){
        case 'N':
            printf("NUM(%d)\n",t->val);
            break;
        
        case 'V':
            printf("ID(%s)\n",t->varname);
            break;
        
        case '+':
            printf("+\n");
            break;
        
        case '-':
            printf("-\n");
            break;
        
        case '*':
            printf("*\n");
            break;

        case '/':
            printf("/\n");
            break;

        case '<':
            printf("<\n");
            break;

        case '>':
            printf(">\n");
            break;

        case NODE_LE:
            printf("<=\n");
            break;

        case NODE_GE:
            printf(">=\n");
            break;

        case NODE_EQ:
            printf("==\n");
            break;
        
        case NODE_NE:
            printf("!=\n");
            break;

        case '=':
            printf("=\n");
            break;
        
        case 'R':
            printf("READ\n");
            break;
        
        case 'W':
            printf("WRITE\n");
            break;
        
        case 'C':
            printf("CONNECTOR\n");
            break;
        
        case 'I':
            printf("IF\n");
            break;
        
        case 'L':
            printf("WHILE\n");
            break;
        
        default:
            printf("UNKNOWN\n");
    }
    printtree(t->left);
    printtree(t->middle);
    printtree(t->right);
}
int yyerror(char *s){
    printf("Error: %s\n", s);
    return 1;
}
int main(){
    yyin = fopen("input.txt", "r");
    yyparse();
    fclose(yyin);

    fp = fopen("out.xsm", "w");

    if(fp == NULL){
        printf("FILE OPEN ERROR\n");
        return 1;
    }
    printf("codegen running\n");
    codegen(root);
    printf("codegen ended\n");

    fclose(fp);
    return 0;
}