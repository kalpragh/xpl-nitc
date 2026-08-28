%{
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"tnode.h"
#include "symboltable.h"
struct tnode *root;

int codegen(struct tnode *t);
int yylex(void);
int yyerror(char *s);
extern FILE *yyin;
FILE *fp;
int currentType;

%}

%union{
    struct tnode *node;
    int type;
}

%token <node> NUM ID
%token START END READ WRITE
%token DECL ENDDECL INT STR
%token IF THEN ELSE ENDIF
%token WHILE DO ENDWHILE
%token BREAK CONTINUE
%token LE GE EQ NE
%token REPEAT UNTIL

%left '<' '>' LE GE EQ NE
%left '+' '-'
%left '*' '/'

%type <node> Program Slist Stmt Expr
%type <type> type

%%

Program:
    declarations START Slist END ';' { root = $3; }
;

declarations:
    DECL declList ENDDECL
    | DECL ENDDECL
;

declList:
    declList declaration
    | declaration
;

declaration:
    type varList ';' 
;

type:
    INT
    {
        currentType = TYPE_INT;
        $$ = TYPE_INT;
    }
    | STR
    {
        currentType = TYPE_STR;
        $$ = TYPE_STR;
    }
;

varList:
    varList ',' ID
    {
        Install($3->varname, currentType, 1);
    }
    | ID
    {
        Install($1->varname, currentType, 1);
    }
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
  | BREAK ';'
    {
        $$ = createtree(0,TYPE_NONE,'B',NULL,NULL,NULL,NULL);
    }
  | CONTINUE ';'
    {
        $$ = createtree(0,TYPE_NONE,'K',NULL,NULL,NULL,NULL);
    }
  | REPEAT Slist UNTIL '(' Expr ')' ';'
    {
        $$= createtree(0,TYPE_NONE,'U',NULL,$5,$2,NULL);
    }
  | DO Slist WHILE '(' Expr ')' ';'
    {
        $$= createtree(0,TYPE_NONE,'D',NULL,$5,$2,NULL);
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

    if (yyin == NULL) {
        printf("Cannot open input.txt\n");
        return 1;
    }

    yyparse();
    fclose(yyin);

    printtree(root);

    return 0;
}