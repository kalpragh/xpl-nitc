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
struct Gsymbol *currentFunction = NULL;
extern struct Gsymbol *Ghead;
void printtree(struct tnode *t);
%}

%union{
    struct tnode *node;
    int type;
    struct Paramstruct *paramlist;
}

%token <node> NUM ID
%token START END READ WRITE
%token DECL ENDDECL INT STR
%token IF THEN ELSE ENDIF
%token WHILE DO ENDWHILE
%token BREAK CONTINUE
%token LE GE EQ NE
%token REPEAT UNTIL
%token RETURN

%left '<' '>' LE GE EQ NE
%left '+' '-'
%left '*' '/'

%type <node> Slist Stmt Expr Array Array2D
%type <type> type
%type <paramlist> ParamList
%type <paramlist> Param
%type <node> ArgList Body

%%

Program:
    GdeclBlock FDefBlock MainBlock
;
LdeclBlock:
    DECL LDecList ENDDECL
  | DECL ENDDECL
;

LDecList:
    LDecList LDecl
  | LDecl
;

LDecl:
    type LIdList ';'
;

LIdList:
    LIdList ',' ID
    {
        LInstallVar($3->varname, currentType, 0);
    }
  | ID
    {
        LInstallVar($1->varname, currentType, 0);
    }
;
GdeclBlock:
    DECL GDeclList ENDDECL
  | DECL ENDDECL
;

GDeclList:
    GDeclList GDecl
  | GDecl
;

GDecl:
    type GidList ';'
;

type:
    INT { currentType = TYPE_INT; $$ = TYPE_INT; }
  | STR { currentType = TYPE_STR; $$ = TYPE_STR; }
;

GidList:
    GidList ',' Gid
  | Gid
;
MainBlock:
    {
        currentFunction = Lookup("main");
        if(currentFunction == NULL){ printf("Error: main not declared\n"); exit(1); }
        if(!currentFunction->isFunction){ printf("Error: main is not a function\n"); exit(1); }
        if(currentFunction->type != TYPE_INT){ printf("Error: main must return int\n"); exit(1); }
        if(currentFunction->paramlist != NULL){ printf("Error: main must take no arguments\n"); exit(1); }
        if(currentFunction->isDefined){ printf("Error: main already defined\n"); exit(1); }
        ClearLocalTable();
        SetCurrentFunction(currentFunction);
    }
    LdeclBlock Body
    {
        currentFunction->funcbody = $3;   /* verify this $N once you paste it in — mid-rule shifts numbering */
        currentFunction->isDefined = 1;
        SetCurrentFunction(NULL);
        currentFunction = NULL;
    }
;
Gid:
    ID
    {
        InstallVar($1->varname, currentType, 1, 0, 0, 0);
    }
  | ID '[' NUM ']'
    {
        if($3->val<=0){ printf("Array size must be positive\n"); exit(1); }
        InstallVar($1->varname, currentType, $3->val, 1, 0, 0);
    }
  | ID '[' NUM ']' '[' NUM ']'
    {
        if($3->val<=0 || $6->val<=0){ printf("array dimensions must be positive\n"); exit(1); }
        InstallVar($1->varname, currentType, $3->val*$6->val, 1, $6->val, 0);
    }
  | '*' ID
    {
        InstallVar($2->varname, currentType, 1, 0, 0, 1);
    }
  | ID '(' ParamList ')'
    {
        InstallFunc($1->varname, currentType, $3);
    }
;
FDefBlock:

  | FDefBlock FDef
  
;
FDef:
    type ID '(' ParamList ')' '{'
    {
        currentFunction = Lookup($2->varname);

        if(currentFunction == NULL)
        {
            printf("Error: function %s not declared\n",
                   $2->varname);
            exit(1);
        }

        if(!currentFunction->isFunction)
        {
            printf("Error: %s is not a function\n",
                   $2->varname);
            exit(1);
        }

        if(currentFunction->type != $1)
        {
            printf("Error: return type mismatch in function %s\n",
                   $2->varname);
            exit(1);
        }

        if(!CheckFunctionDefinition(currentFunction, $4))
            exit(1);

        if(currentFunction->isDefined)
        {
            printf("Error: function %s already defined\n",
                   $2->varname);
            exit(1);
        }

        ClearLocalTable();
        SetCurrentFunction(currentFunction);
        InstallParams($4);
    }
    LdeclBlock Body
    '}'
    {
        currentFunction->funcbody = $9;
        currentFunction->isDefined = 1;
        ClearLocalTable();
        SetCurrentFunction(NULL);
        currentFunction = NULL;
    }
;
Body:
    START Slist RETURN Expr ';' END
    {
        struct tnode *retnode = createtree(0, TYPE_NONE, NODE_RETURN, NULL, $4, NULL, NULL);
        $$ = createtree(0, TYPE_NONE, 'C', NULL, $2, NULL, retnode);
    }
;
ParamList:
    /* empty */        { $$ = NULL; }
  | Param               { $$ = $1; }
  | ParamList ',' Param { $$ = appendParam($1, $3); }
;

Param:
    INT ID { $$ = makeParam(TYPE_INT, $2->varname); }
  | STR ID { $$ = makeParam(TYPE_STR, $2->varname); }
;
Slist:
    Slist Stmt { $$ = createtree(0,TYPE_NONE,'C',NULL,$1,NULL,$2); }
  | Stmt       { $$ = $1; }
;

Stmt:
    ID '=' Expr ';'
    {
        struct Lsymbol *lentry = NULL;
        struct Gsymbol *gentry = NULL;

        int found = LookupVar($1->varname, &lentry, &gentry);

        if(found == 0)
        {
            printf("Variable %s not declared\n", $1->varname);
            exit(1);
        }

        if(found == 2 && gentry->isArray)
        {
            printf("Cannot use array %s as a scalar\n", $1->varname);
            exit(1);
        }

        struct tnode *idnode =createtree(0, TYPE_NONE, 'V',$1->varname, NULL, NULL, NULL);

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
   | READ '(' '*' ID ')' ';'
    {
        struct Gsymbol *entry = Lookup($4->varname);
        if(entry == NULL){
            printf("Variable %s not declared\n", $4->varname);
            exit(1);
        }
        if(!entry->isPointer){
            printf("%s is not a pointer\n", $4->varname);
            exit(1);
        }
        struct tnode *idnode = createtree(0, TYPE_NONE, 'V', $4->varname, NULL, NULL, NULL);
        struct tnode *derefnode = createtree(0, TYPE_NONE, NODE_DEREF, NULL, idnode, NULL, NULL);
        $$ = createtree(0, TYPE_NONE, 'R', NULL, derefnode, NULL, NULL);
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
  | '*' ID '=' Expr ';'
    {
        struct Gsymbol *entry = Lookup($2->varname);
        if(entry == NULL){
            printf("Variable %s not declared\n", $2->varname);
            exit(1);
        }
        if(!entry->isPointer){
            printf("%s is not a pointer\n", $2->varname);
            exit(1);
        }
        struct tnode *idnode = createtree(0, TYPE_NONE, 'V', $2->varname, NULL, NULL, NULL);
        struct tnode *derefnode = createtree(0, TYPE_NONE, NODE_DEREF, NULL, idnode, NULL, NULL);
        $$ = createtree(0, TYPE_NONE, '=', NULL, derefnode, NULL, $4);
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
        struct Lsymbol *lentry = NULL;
        struct Gsymbol *gentry = NULL;
        int found = LookupVar($1->varname, &lentry, &gentry);
        if(found == 0){
            printf("Variable %s not declared\n", $1->varname);
            exit(1);
        }
        if(found == 2 && gentry->isArray){
            printf("cannot use array %s as a scalar\n", $1->varname);
            exit(1);
        }
        $$ = createtree(0,TYPE_NONE,'V',$1->varname,NULL,NULL,NULL);
    }
  | Array { $$ = $1; }
  | Array2D { $$=$1; }
  | '&' ID
  {
    struct Gsymbol *entry=Lookup($2->varname);
    if(entry==NULL){
        printf("Variable %s not declared\n",$2->varname);
        exit(1);
    }
    struct tnode *idnode=createtree(0,TYPE_NONE,'V',$2->varname,NULL,NULL,NULL);
    $$=createtree(0,TYPE_NONE,NODE_ADDR,NULL,idnode,NULL,NULL);
  }
  | '*' ID
  {
    struct Gsymbol *entry=Lookup($2->varname);
    if(entry==NULL){
        printf("Variable %s not declared\n",$2->varname);
        exit(1);
    }
    if(!entry->isPointer){
        printf("%s is not a pointer\n",$2->varname);
        exit(1);
    }
    struct tnode *idnode=createtree(0,TYPE_NONE,'V',$2->varname,NULL,NULL,NULL);
    $$=createtree(0,TYPE_NONE,NODE_DEREF,NULL,idnode,NULL,NULL);
  }
  | ID '(' ArgList ')'
  {
    $$=createtree(0,TYPE_NONE,NODE_CALL,$1->varname,NULL,NULL,$3);
  }
;
ArgList:
    ArgList ',' Expr { $$ = appendArg($1, makeArgNode($3)); }
  | Expr { $$ = makeArgNode($1); }
  | { $$ = NULL; }
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

        case NODE_CALL:
            printf("CALL(%s)\n", t->varname);
            break;

        case NODE_RETURN:
            printf("RETURN\n");
            break;

        case NODE_ARRAY:
            printf("ARRAY\n");
            break;

        case NODE_ARRAY2D:
            printf("ARRAY2D\n");
            break;

        case NODE_ADDR:
            printf("ADDRESS\n");
            break;

        case NODE_DEREF:
            printf("DEREFERENCE\n");
            break;

        case 'B':
            printf("BREAK\n");
            break;

        case 'K':
            printf("CONTINUE\n");
            break;

        case 'U':
            printf("REPEAT-UNTIL\n");
            break;

        case 'D':
            printf("DO-WHILE\n");
            break;
        
        case NODE_ARG:
            printf("ARG\n");
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

     printf("\n===== GLOBAL SYMBOL TABLE =====\n");
    PrintSymbolTable();

    printf("\n===== FUNCTION ASTs =====\n");

    struct Gsymbol *temp = Ghead;

    while(temp != NULL)
    {
        if(temp->isFunction && temp->funcbody != NULL)
        {
            printf("\nAST for function %s:\n", temp->name);
            printtree(temp->funcbody);
        }

        temp = temp->next;
    }

    return 0;
}