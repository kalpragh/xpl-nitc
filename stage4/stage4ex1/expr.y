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

%type <node> Program Slist Stmt Expr Array Array2D
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
    varList ',' ID '[' NUM ']'
    {
        if($5->val<=0){
            printf("Array size must be positive\n");
            exit(1);
        }
        Install($3->varname,currentType,$5->val,1,0);
    }
    | varList ',' ID 
    {
        Install($3->varname, currentType, 1, 0,0);
    }
    | ID '[' NUM ']' '[' NUM ']'
    {
        if($3->val<=0 || $6->val<=0){
            printf("array dimensions must be positive\n");
            exit(1);
        }
        Install($1->varname,currentType,$3->val*$6->val,1,$6->val);
    }
    | ID '[' NUM ']'
    {
        if($3->val<=0){
            printf("Array size must be positive\n");
            exit(1);
        }
        Install($1->varname, currentType, $3->val, 1,0);
    }
    | ID 
    {
        Install($1->varname, currentType,1,0,0);
    }
;

Slist:
    Slist Stmt { $$ = createtree(0,TYPE_NONE,'C',NULL,$1,NULL,$2); }
  | Stmt       { $$ = $1; }
;

Stmt:
    ID '=' Expr ';'
    {
        struct Gsymbol *entry = Lookup($1->varname);
        if(entry == NULL){
            printf("Variable %s not declared\n", $1->varname);
            exit(1);
        }
        if(entry->isArray){
            printf("Cannot use array %s as a scalar\n", $1->varname);
            exit(1);
        }
        struct tnode *idnode = createtree(0, TYPE_NONE, 'V', $1->varname, NULL, NULL, NULL);

        $$ = createtree(0, TYPE_NONE, '=', NULL, idnode, NULL, $3);
    }
      | ID '[' Expr ']' '[' Expr ']' '=' Expr ';'
   {
        struct Gsymbol *entry=Lookup($1->varname);
        if(entry==NULL){ 
            printf("variable %s not declared\n",$1->varname);
            exit(1);
        }
        if(entry->cols==0){ 
            printf("%s is not a 2d array\n",$1->varname);
            exit(1);
        }
        int rows=entry->size/entry->cols;
        if($3->nodetype=='N' && ($3->val<0 || $3->val>=rows)){
            printf("Row index out of bounds\n");
            exit(1);
        }
        if($6->nodetype=='N' && ($6->val<0 || $6->val>=entry->cols)){
            printf("Col index out of bounds\n");
            exit(1);
        }
        struct tnode *idnode=createtree(0,TYPE_NONE,'V',$1->varname,NULL,NULL,NULL);
        struct tnode *arrnode=createtree(0,TYPE_NONE,NODE_ARRAY2D,NULL,idnode,$3,$6);
        $$=createtree(0,TYPE_NONE,'=',NULL,arrnode,NULL,$9);
   }
   | ID '[' Expr ']' '=' Expr ';'
    {
        struct Gsymbol *entry = Lookup($1->varname);
        if(entry == NULL){
            printf("Variable %s not declared\n", $1->varname);
            exit(1);
        }
        if(!entry->isArray){
            printf("%s is a scalar; cannot index it\n", $1->varname);
            exit(1);
        }
        if($3->nodetype=='N'){
            if($3->val < 0 || $3->val >= entry->size){
                printf("Array index out of bounds\n");
                exit(1);
            }
        }
        struct tnode *idnode = createtree(0, TYPE_NONE, 'V', $1->varname, NULL, NULL, NULL);
        struct tnode *arrnode =  createtree(0, TYPE_NONE, NODE_ARRAY,NULL, idnode, NULL, $3);
        $$ = createtree(0, TYPE_NONE, '=',NULL, arrnode, NULL, $6);
    }
  | READ '(' ID ')' ';'
    {
        struct Gsymbol *entry = Lookup($3->varname);
        if(entry == NULL){
            printf("Variable %s not declared\n", $3->varname);
            exit(1);
        }
        if(entry->isArray){
            printf("cannot read array %s as a scalar\n", $3->varname);
            exit(1);
        }
        struct tnode *idnode = createtree(0, TYPE_NONE, 'V', $3->varname, NULL, NULL, NULL);

        $$ = createtree(0, TYPE_NONE, 'R', NULL, idnode, NULL, NULL);
    }
  | READ '(' ID '[' Expr ']' ')' ';'
    {
        struct Gsymbol *entry = Lookup($3->varname);
        if(entry == NULL){
            printf("Variable %s not declared\n", $3->varname);
            exit(1);
        }
        if(!entry->isArray){
            printf("%s is a scalar; cannot index it\n", $3->varname);
            exit(1);
        }
        if($5->nodetype=='N'){
            if($5->val < 0 || $5->val >= entry->size){
                printf("Array index out of bounds\n");
                exit(1);
            }
        }
        struct tnode *idnode = createtree(0, TYPE_NONE, 'V',$3->varname, NULL, NULL, NULL);
        struct tnode *arrnode =createtree(0, TYPE_NONE, NODE_ARRAY,NULL, idnode, NULL, $5);
        $$ = createtree(0, TYPE_NONE, 'R',NULL, arrnode, NULL, NULL);
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
  | ID
    {
        struct Gsymbol *entry = Lookup($1->varname);
        if(entry == NULL){
            printf("Variable %s not declared\n", $1->varname);
            exit(1);
        }
        if(entry->isArray){
            printf("cannot use array %s as a scalar\n", $1->varname);
            exit(1);
        }
        $$ = createtree(0,TYPE_NONE,'V',$1->varname,NULL,NULL,NULL);
    }
  | Array { $$ = $1; }
  | Array2D { $$=$1; }
;

Array:
    ID '[' Expr ']'
    {
        struct Gsymbol *entry = Lookup($1->varname);
        if(entry == NULL){
            printf("variable %s not declared\n", $1->varname);
            exit(1);
        }
        if(!entry->isArray){
            printf("%s is a scalar, cannot index it\n", $1->varname);
            exit(1);
        }
        if($3->nodetype=='N'){
            if($3->val < 0 || $3->val >= entry->size){
                printf("array index out of bounds\n");
                exit(1);
            }
        }
        struct tnode *idnode =createtree(0, TYPE_NONE, 'V',$1->varname, NULL, NULL, NULL);
        $$ = createtree(0, TYPE_NONE, NODE_ARRAY, NULL, idnode, NULL, $3);
    }
;

Array2D:
    ID '[' Expr ']' '[' Expr ']'
    {
        struct Gsymbol *entry=Lookup($1->varname);
        if(entry==NULL){
            printf("variable %s not declared\n",$1->varname);
            exit(1);
        }
        if(entry->cols==0){
            printf("%s is not a 2d array\n",$1->varname);
            exit(1);
        }
        int rows=entry->size/entry->cols;
        if($3->nodetype=='N' && ($3->val<0 || $3->val>=rows)){
            printf("Row index out of bounds\n");
            exit(1);
        }
        if($6->nodetype=='N' && ($6->val<0 || $6->val>=entry->cols)){
            printf("Col index out of bounds\n");
            exit(1);
        }
        struct tnode *idnode=createtree(0,TYPE_NONE,'V',$1->varname,NULL,NULL,NULL);
        struct tnode *rownode=createtree(0,TYPE_NONE,NODE_ARRAY2D,NULL,idnode,$3,$6);
        $$=rownode;
    }
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

int main(int argc, char *argv[]){
    char *infile = "input.txt";
    if(argc > 1) infile = argv[1];

    yyin = fopen(infile, "r");
    if(yyin == NULL){
        printf("Error: could not open input file %s\n", infile);
        exit(1);
    }
    yyparse();
    fclose(yyin);
    fp = fopen("output.xsm", "w");
    writeHeader();
    codegen(root);
    writeArrayErrorHandler();
    fprintf(fp, "HALT\n");
    fclose(fp);
    return 0;
}