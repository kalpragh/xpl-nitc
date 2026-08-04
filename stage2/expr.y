%{
#include<stdio.h>
#include<stdlib.h>
#include"tnode.h"

tnode *root;
int yyerror(const char *s);
int yylex(void);
%}

%union{
    int num;
    char id;
    struct tnode *node;
}

%token<num>NUM
%token<id>ID
%token START END READ WRITE

%left '+'
%left '*'

%type <node> Program Slist Stmt Expr

%%
Program:
    START Slist END {root=$2;}
;
Slist:
    Slist Stmt {$$=createtree(0,'C',NULL,$1,$2); }
    | Stmt {$$=$1;}
;

Stmt:
    ID '=' Expr ';'
    {
        $$ = createtree(0,'=',NULL,
                createtree(0, ID, (char[]){$1,'\0'}, NULL, NULL),
                $3);
    }

  | READ '(' ID ')' ';'
    {
        $$ = createtree(0, READ, NULL,
                createtree(0, ID, (char[]){$3,'\0'}, NULL, NULL),
                NULL);
    }

  | WRITE '(' Expr ')' ';'
    {
        $$ = createtree(0, WRITE, NULL, $3, NULL);
    }
;
Expr:
      Expr '+' Expr {$$=createtree(0,'+',NULL,$1,$3);}
    | Expr '*' Expr {$$=createtree(0,'*',NULL,$1,$3);}
    | NUM {$$=createtree($1,NUM,NULL,NULL,NULL); }
    | ID  {$$=createtree(0,ID,(char[]){$1,'\0'},NULL,NULL); }
    ;

%%
void preorder(struct tnode* t) {
    if (!t) return;

    if (t->nodetype == NUM)
        printf("%d ", t->val);
    else if (t->nodetype == ID)
        printf("%s ", t->varname);
    else if (t->nodetype == READ)
        printf("READ ");
    else if (t->nodetype == WRITE)
        printf("WRITE ");
    else if (t->nodetype == 'C')
        printf("C ");
    else
        printf("%c ", t->nodetype);

    preorder(t->left);
    preorder(t->right);
}
int main (){
    yyparse();
    printf("parsing done. AST created.\n");
    preorder(root);  
    printf("\n");
    return 0;
}
int yyerror(char const *s){
    printf("yyerror %s\n",s);
    return 1;
}