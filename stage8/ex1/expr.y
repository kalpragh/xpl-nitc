%{
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"tnode.h"
#include "symboltable.h"
#include "classtable.h"
struct Classtable *currentClass = NULL;
struct tnode *root;
void codegenFunction(struct Gsymbol *f);
int codegen(struct tnode *t);
int yylex(void);
int yyerror(char *s);
extern FILE *yyin;
extern int nextBinding;
FILE *fp;
struct Typetable *currentType;
struct Gsymbol *currentFunction = NULL;
struct TupleType *currentTuple = NULL;
extern struct Gsymbol *Ghead;
void printtree(struct tnode *t);
void noClassType(const char *what){
    if(isClassType(currentType)){ printf("Error: %s of class type are not supported\n", what); exit(1); }
}
void beginMethodDef(struct Classtable *c, char *mname, struct Typetable *type, struct Paramstruct *params){
    struct Memberfunclist *m = Class_MlookupSig(c, mname, params);
    if(m == NULL){ printf("Error: method %s with these parameter types is not declared in class %s\n", mname, c->name); exit(1); }
    if(m->func->owner != c){
        printf("Error: method %s is inherited by class %s and not redeclared, so it cannot be defined there\n", mname, c->name);
        exit(1);
    }
    currentFunction = m->func;
    if(currentFunction->type != type){ printf("Error: return type mismatch in method %s\n", mname); exit(1); }
    if(!CheckFunctionDefinition(currentFunction, params)) exit(1);
    if(currentFunction->isDefined){ printf("Error: method %s already defined\n", mname); exit(1); }
    ClearLocalTable();
    SetCurrentFunction(currentFunction);
    InstallParams(params);
}
struct TupleType *needTuple(char *name){
    struct TupleType *t = LookupTupleType(name);
    if(t == NULL){ printf("Error: unknown tuple type %s\n", name); exit(1); }
    return t;
}
struct Typetable *currentUType = NULL;

struct tnode *scalarVarNode(char *name){
    struct Lsymbol *lentry = NULL; struct Gsymbol *gentry = NULL;
    int found = LookupVar(name, &lentry, &gentry);
    if(found == 0){ printf("Variable %s not declared\n", name); exit(1); }
    if(found == 2 && gentry->isArray){ printf("cannot use array %s as a scalar\n", name); exit(1); }
    struct tnode *n = createtree(0, TYPE_NONE, 'V', name, NULL, NULL, NULL);
    n->type = (found == 1) ? lentry->type : gentry->type;
    return n;
}

void noUserType(const char *what){
    if(isUserType(currentType)){ printf("Error: %s of a user-defined type are not supported\n", what); exit(1); }
}
void beginFuncDef(char *fname, struct Typetable *type, int isPtr, struct TupleType *tt, struct Paramstruct *params){
    currentFunction = Lookup(fname);
    if(currentFunction == NULL){ printf("Error: function %s not declared\n", fname); exit(1); }
    if(!currentFunction->isFunction){ printf("Error: %s is not a function\n", fname); exit(1); }
    if(currentFunction->type != type || currentFunction->isPointer != isPtr || currentFunction->ttype != tt){
        printf("Error: return type mismatch in function %s\n", fname); exit(1);
    }
    if(!CheckFunctionDefinition(currentFunction, params)) exit(1);
    if(currentFunction->isDefined){ printf("Error: function %s already defined\n", fname); exit(1); }
    ClearLocalTable();
    SetCurrentFunction(currentFunction);
    InstallParams(params);
}
void endFuncDef(struct tnode *body){
    currentFunction->funcbody = body;
    currentFunction->locals = Lhead;
    currentFunction->numLocals = GetLocalCount();
    currentFunction->isDefined = 1;
    SetCurrentFunction(NULL);
    currentFunction = NULL;
}
%}

%union{
    struct tnode *node;
    struct Typetable *type;
    struct Paramstruct *paramlist;
    struct Field *field;
    struct TupleType *tuple;
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
%token TUPLE
%token TYPE ENDTYPE NULLT ALLOC FREE INITIALIZE
%token CLASS ENDCLASS EXTENDS NEW DELETE SELF
%type <node> MethodCall
%token <node> STRING
%type <field> FieldList Field

%left '<' '>' LE GE EQ NE
%left '+' '-'
%left '*' '/'

%type <node> Slist Stmt Expr Array Array2D FieldRef UVar 
%type <type> type
%type <paramlist> ParamList
%type <paramlist> Param
%type <node> ArgList Body

%%

Program:
    TypeDefBlock ClassDefBlock GdeclBlock FDefBlock 
;
ClassDefBlock: 
    | CLASS ClassDefList ENDCLASS   { ReserveVtables(); }
;
ClassDefList: ClassDefList ClassDef | ClassDef ;
ClassDef: Cname '{' CBody MDefList '}'   { currentClass = NULL; } ;
Cname:
    ID              { currentClass = CInstall($1->varname, NULL); }
  | ID EXTENDS ID   { currentClass = CInstall($1->varname, $3->varname); }
;
CBody: DECL CMemberList ENDDECL 
    | DECL ENDDECL 
;
CMemberList: CMemberList CMember 
    | CMember 
;
CMember:
    type ID ';'                        { Class_Finstall(currentClass, $1, $2->varname); }
  | type ID '(' ParamList ')' ';'      { Class_Minstall(currentClass, $2->varname, $1, $4); }
;
MDefList: 
    | MDefList MDef 
;
MDef:
    type ID '(' ParamList ')' '{'
    { beginMethodDef(currentClass, $2->varname, $1, $4); }
    LdeclBlock Body '}'
    { endFuncDef($9); }
;
TypeDefBlock: 
            | TYPE TypeDefList ENDTYPE 
;
TypeDefList: TypeDefList TypeDef 
            | TypeDef 
;
TypeDef: TypeDefHead '{' UFieldList '}' 
;
TypeDefHead: ID { currentUType = TInstall($1->varname); } ;
UFieldList: UFieldList UField 
            | UField ;
UField:
    INT ID ';' { FInstall(currentUType, $2->varname, TYPE_INT); }
  | STR ID ';' { FInstall(currentUType, $2->varname, TYPE_STR); }
  | ID  ID ';' { FInstall(currentUType, $2->varname, TResolve($1->varname)); }
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
  | LTupleHead LTVarList ';'
;

LTupleHead:
    TUPLE ID
    {
        currentTuple = LookupTupleType($2->varname);
        if(currentTuple == NULL){ printf("Error: unknown tuple type %s\n", $2->varname); exit(1); }
    }
;

LTVarList:
    LTVarList ',' LTVar
  | LTVar
;

LTVar:
    ID     { LInstallTuple($1->varname, currentTuple, 0, 0); }
  | '*' ID { LInstallTuple($2->varname, currentTuple, 1, 0); }
;
LIdList:
    LIdList ',' ID
    {
        LInstallVar($3->varname, currentType, 0, 0);
    }
  | LIdList ',' '*' ID
    {
        noUserType("pointers");
        LInstallVar($4->varname, currentType, 1, 0);
    }
  | ID
    {
        LInstallVar($1->varname, currentType, 0, 0);
    }
  | '*' ID
    {
        noUserType("pointers");
        LInstallVar($2->varname, currentType, 1, 0);
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
  | TupleHead GTVarList ';'
;

TupleHead:
    TUPLE ID '(' FieldList ')'
    {
        currentTuple = InstallTupleType($2->varname, $4);
    }
  | TUPLE ID
    {
        currentTuple = LookupTupleType($2->varname);
        if(currentTuple == NULL){ printf("Error: unknown tuple type %s\n", $2->varname); exit(1); }
    }
;

FieldList:
    FieldList ',' Field { $$ = appendField($1, $3); }
  | Field               { $$ = $1; }
;

Field:
    INT ID { $$ = makeField(TYPE_INT, $2->varname); }
  | STR ID { $$ = makeField(TYPE_STR, $2->varname); }
;

GTVarList:
    GTVarList ',' GTVar
  | GTVar
;

GTVar:
    ID     { InstallTupleVar($1->varname, currentTuple, 0); }
  | '*' ID { InstallTupleVar($2->varname, currentTuple, 1); }
  | ID '(' ParamList ')'
    { InstallTupleFunc($1->varname, currentTuple, 0, $3); }
  | '*' ID '(' ParamList ')'
    { InstallTupleFunc($2->varname, currentTuple, 1, $4); }
;

type:
    INT { currentType = TYPE_INT; $$ = TYPE_INT; }
  | STR { currentType = TYPE_STR; $$ = TYPE_STR; }
  | ID  { currentType = $$ = TResolve($1->varname); }
;

GidList:
    GidList ',' Gid
  | Gid
;

Gid:
    ID
    {
        InstallVar($1->varname, currentType, 1, 0, 0, 0);
    }
  | ID '[' NUM ']'
    {
        noClassType("arrays");
        if($3->val<=0){ printf("Array size must be positive\n"); exit(1); }
        InstallVar($1->varname, currentType, $3->val, 1, 0, 0);
    }
  | ID '[' NUM ']' '[' NUM ']'
    {
        noUserType("arrays");
        if($3->val<=0 || $6->val<=0){ printf("array dimensions must be positive\n"); exit(1); }
        InstallVar($1->varname, currentType, $3->val*$6->val, 1, $6->val, 0);
    }
  | '*' ID
    {
        noUserType("pointers");
        InstallVar($2->varname, currentType, 1, 0, 0, 1);
    }
  | ID '(' ParamList ')'
    {
        noClassType("functions returning objects");
        InstallFunc($1->varname, currentType, $3);
    }
  | '*' ID '(' ParamList ')'
    {
        noUserType("pointer-returning functions");
        InstallFunc($2->varname, currentType, $4);

        struct Gsymbol *f = Lookup($2->varname);
        f->isPointer = 1;
    }
;
FDefBlock:

  | FDefBlock FDef
  
  
;
FDef:
    type ID '(' ParamList ')' '{'
    {
        if(Lookup($2->varname) == NULL && strcmp($2->varname, "main") == 0)
        InstallFunc("main", $1, $4);
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
        currentFunction->locals=Lhead;
        currentFunction->numLocals=GetLocalCount();
        currentFunction->isDefined = 1;
         
        SetCurrentFunction(NULL);
        currentFunction = NULL;
    }
    | type '*' ID '(' ParamList ')' '{'
    {
        currentFunction = Lookup($3->varname);

        if(currentFunction == NULL)
        {
            printf("Error: function %s not declared\n",
                   $3->varname);
            exit(1);
        }

        if(!currentFunction->isFunction)
        {
            printf("Error: %s is not a function\n",
                   $3->varname);
            exit(1);
        }

        if(currentFunction->type != $1 ||
           !currentFunction->isPointer)
        {
            printf("Error: return type mismatch in function %s\n",
                   $3->varname);
            exit(1);
        }

        if(!CheckFunctionDefinition(currentFunction, $5))
            exit(1);

        if(currentFunction->isDefined)
        {
            printf("Error: function %s already defined\n",
                   $3->varname);
            exit(1);
        }

        ClearLocalTable();
        SetCurrentFunction(currentFunction);
        InstallParams($5);
    }
    LdeclBlock Body
    '}'
    {
        currentFunction->funcbody = $10;
        currentFunction->locals = Lhead;
        currentFunction->numLocals = GetLocalCount();
        currentFunction->isDefined = 1;

        SetCurrentFunction(NULL);
        currentFunction = NULL;
    }
    | TUPLE ID ID '(' ParamList ')' '{'
    {
        beginFuncDef($3->varname, TYPE_TUPLE, 0, needTuple($2->varname), $5);
    }
    LdeclBlock Body '}'
    {
        endFuncDef($10);
    }
    | TUPLE ID '*' ID '(' ParamList ')' '{'
    {
        beginFuncDef($4->varname, TYPE_TUPLE, 1, needTuple($2->varname), $6);
    }
    LdeclBlock Body '}'
    {
        endFuncDef($11);
    }
;
Body:
    START Slist RETURN Expr ';' END
    {
        struct tnode *retnode = createtree(0, TYPE_NONE, NODE_RETURN, NULL, $4, NULL, NULL);
        $$ = createtree(0, TYPE_NONE, 'C', NULL, $2, NULL, retnode);
    }
  | START RETURN Expr ';' END
    {
        $$ = createtree(0, TYPE_NONE, NODE_RETURN, NULL, $3, NULL, NULL);
    }
;
ParamList:
    /* empty */        { $$ = NULL; }
  | Param               { $$ = $1; }
  | ParamList ',' Param { $$ = appendParam($1, $3); }
;

Param:
    INT ID { $$ = makeParam(TYPE_INT, $2->varname,0); }
  | STR ID { $$ = makeParam(TYPE_STR, $2->varname,0); }
  | INT '*' ID {$$=makeParam(TYPE_INT,$3->varname,1); }
  | STR '*' ID {$$=makeParam(TYPE_STR,$3->varname,1); }
  | TUPLE ID ID      { $$ = makeTupleParam($2->varname, $3->varname, 0); }
  | TUPLE ID '*' ID  { $$ = makeTupleParam($2->varname, $4->varname, 1); }
  | ID ID { $$ = makeParam(TResolve($1->varname), $2->varname, 0); }
   
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
        struct Lsymbol *lentry = NULL;
        struct Gsymbol *gentry = NULL;
        int found = LookupVar($3->varname, &lentry, &gentry);
        if(found == 0){
            printf("Variable %s not declared\n", $3->varname);
            exit(1);
        }
        if(found == 2 && gentry->isArray){
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
        struct Lsymbol *lentry=NULL; struct Gsymbol *gentry=NULL;
        int found = LookupVar($4->varname, &lentry, &gentry);
        if(found==0){
            printf("Variable %s not declared\n", $4->varname);
            exit(1);
        }
        int isptr = (found==1) ? lentry->isPointer : gentry->isPointer;
        if(!isptr){
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
        struct Lsymbol *lentry=NULL; struct Gsymbol *gentry=NULL;
        int found = LookupVar($2->varname, &lentry, &gentry);
        if(found==0){
            printf("Variable %s not declared\n", $2->varname);
            exit(1);
        }
        int isptr = (found==1) ? lentry->isPointer : gentry->isPointer;
        if(!isptr){
            printf("%s is not a pointer\n", $2->varname);
            exit(1);
        }
        struct tnode *idnode = createtree(0, TYPE_NONE, 'V', $2->varname, NULL, NULL, NULL);
        struct tnode *derefnode = createtree(0, TYPE_NONE, NODE_DEREF, NULL, idnode, NULL, NULL);
        $$ = createtree(0, TYPE_NONE, '=', NULL, derefnode, NULL, $4);
    }
  | ID '(' ArgList ')' ';'
    {
        $$ = createtree(0, TYPE_NONE, NODE_CALL, $1->varname, NULL, NULL, $3);
    }
  | FieldRef '=' Expr ';'
    { $$ = createtree(0, TYPE_NONE, '=', NULL, $1, NULL, $3); }
  | READ '(' FieldRef ')' ';'
    { $$ = createtree(0, TYPE_NONE, 'R', NULL, $3, NULL, NULL); }
  | ID '=' ALLOC '(' ')' ';'
    { $$ = makeAllocNode(scalarVarNode($1->varname)); }
  | ID '[' Expr ']' '=' ALLOC '(' ')' ';'
    {
        struct Gsymbol *entry = Lookup($1->varname);
        if(entry == NULL){ printf("Variable %s not declared\n", $1->varname); exit(1); }
        if(!entry->isArray){ printf("%s is a scalar; cannot index it\n", $1->varname); exit(1); }
        if($3->nodetype=='N' && ($3->val < 0 || $3->val >= entry->size)){ printf("Array index out of bounds\n"); exit(1); }
        struct tnode *idnode = createtree(0, TYPE_NONE, 'V', $1->varname, NULL, NULL, NULL);
        struct tnode *arrnode = createtree(0, TYPE_NONE, NODE_ARRAY, NULL, idnode, NULL, $3);
        $$ = makeAllocNode(arrnode);
    }
  | FieldRef '=' ALLOC '(' ')' ';'
    { $$ = makeAllocNode($1); }
  | FREE '(' UVar ')' ';'
    { $$ = makeFreeNode($3); }
  | INITIALIZE '(' ')' ';'
    { $$ = makeInitNode(); }
  | MethodCall ';'                     { $$ = $1; }
  | ID '=' NEW '(' ID ')' ';'          { $$ = makeNewNode(scalarVarNode($1->varname), $5->varname); }
  | FieldRef '=' NEW '(' ID ')' ';'    { $$ = makeNewNode($1, $5->varname); }
  | DELETE '(' UVar ')' ';'            { $$ = makeDeleteNode($3); }
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
        struct Lsymbol *lentry=NULL; struct Gsymbol *gentry=NULL;
        int found = LookupVar($2->varname, &lentry, &gentry);
        if(found==0){
            printf("Variable %s not declared\n",$2->varname);
            exit(1);
        }
        struct tnode *idnode=createtree(0,TYPE_NONE,'V',$2->varname,NULL,NULL,NULL);
        $$=createtree(0,TYPE_NONE,NODE_ADDR,NULL,idnode,NULL,NULL);
  }
  | '*' ID
  {
        struct Lsymbol *lentry=NULL; struct Gsymbol *gentry=NULL;
        int found = LookupVar($2->varname, &lentry, &gentry);
        if(found==0){
            printf("Variable %s not declared\n",$2->varname);
            exit(1);
        }
        int isptr = (found==1) ? lentry->isPointer : gentry->isPointer;
        if(!isptr){
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
  | FieldRef             { $$ = $1; }
  | MethodCall           { $$ = $1; }
  | NULLT                { $$ = makeNullNode(); }
  | FREE '(' UVar ')'    { $$ = makeFreeNode($3); }
  | INITIALIZE '(' ')'   { $$ = makeInitNode(); }
  | STRING               { $$ = $1; }
;

FieldRef:
    ID '.' ID        { $$ = makeFieldNode(scalarVarNode($1->varname), $3->varname); }
  | SELF '.' ID      { $$ = makeFieldNode(makeSelfNode(), $3->varname); }
  | Array '.' ID     { $$ = makeFieldNode($1, $3->varname); }
  | FieldRef '.' ID  { $$ = makeFieldNode($1, $3->varname); }
  
;
MethodCall:
    SELF '.' ID '(' ArgList ')'      { $$ = makeMethodCallNode(makeSelfNode(), $3->varname, $5); }
  | ID '.' ID '(' ArgList ')'        { $$ = makeMethodCallNode(scalarVarNode($1->varname), $3->varname, $5); }
  | FieldRef '.' ID '(' ArgList ')'  { $$ = makeMethodCallNode($1, $3->varname, $5); }
;
UVar:
    ID        { $$ = scalarVarNode($1->varname); }
  | FieldRef  { $$ = $1; }
  | Array     { $$ = $1; }
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
int treeDepth = 0;
void printtree(struct tnode *t){
    if(t==NULL)return;
    for(int i = 0; i < treeDepth; i++) printf("  ");
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

        case NODE_FIELD:
            if(t->field) printf("FIELD(%s)\n", t->field->name);
            else         printf("FIELD(%d)\n", t->val);
            break;
        case NODE_NULL:  
            printf("NULL\n");       
            break;
        case NODE_ALLOC: 
            printf("ALLOC\n");      
            break;
        case NODE_FREE:  
            printf("FREE\n");       
            break;
        case NODE_INIT:  
            printf("INITIALIZE\n"); 
            break;
        case NODE_STRCONST:
            printf("STRING(%s)\n", t->varname);
            break;
        case NODE_METHOD:
            printf("METHOD(%s)\n", t->varname); 
            break;
        case NODE_NEW:    
            printf("NEW\n");    
            break;
        case NODE_DELETE: 
            printf("DELETE\n"); 
            break;
        default:
            printf("UNKNOWN\n");
    }
    treeDepth++;
    printtree(t->left);
    printtree(t->middle);
    printtree(t->right);
    treeDepth--;
}
int yyerror(char *s){
    printf("Error: %s\n", s);
    return 1;
}
int main(int argc, char *argv[]){
    libMode = (argc > 2 && strcmp(argv[2], "-lib") == 0);
    yyin = fopen(argv[1], "r");

    if(yyin == NULL){
        printf("Cannot open input file\n");
        return 1;
    }

    if(libMode) nextBinding = 1024;
    TypeTableCreate();
    if(yyparse() != 0) return 1;
    TypeTablePrint();
    ClassTablePrint();
    for(struct Gsymbol *g = Ghead; g != NULL; g = g->next){
        if(g->isFunction && !g->isDefined){
            printf("Error: function %s declared but not defined\n", g->name);
            return 1;
        }
    }
    if(argc > 2 && strcmp(argv[2], "-ast") == 0){
        for(struct Gsymbol *g = Ghead; g != NULL; g = g->next)
            if(g->isFunction){
                printf("AST of %s:\n", g->name);
                printtree(g->funcbody);
            }
        return 0;
    }
    if(libMode){
        fp = fopen("libcode.xsm", "w");
        writeLibraryDispatcher();
        for(struct Gsymbol *lg = Ghead; lg != NULL; lg = lg->next)
            if(lg->isFunction) codegenFunction(lg);
        writeArrayErrorHandler();
        fclose(fp);
        return 0;
    }

    struct Gsymbol *mf = Lookup("main");
    if(mf == NULL || !mf->isFunction){ printf("Error: main not found\n"); return 1; }
    if(mf->type != TYPE_INT){ printf("Error: main must return int\n"); return 1; }
    if(mf->paramlist != NULL){ printf("Error: main must take no arguments\n"); return 1; }

    struct Gsymbol *mainFn = mf;

    fp = fopen("output.xsm", "w");

    writeHeader();

    WriteVtableInit(fp);

    fprintf(fp, "JMP F%d\n", mainFn->flabel);

    struct Gsymbol *g = Ghead;

    while(g != NULL){
        if(g->isFunction)
            codegenFunction(g);

        g = g->next;
    }

    writeArrayErrorHandler();

    fprintf(fp, "HALT\n");

    fclose(fp);

    return 0;
}