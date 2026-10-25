#include <stdlib.h>
#include <string.h>
#include "tnode.h"
#include <stdio.h>
#include "symboltable.h"
#include "classtable.h"
extern FILE *fp;
#define MAX_LOOP_DEPTH 100
int loopStartStack[MAX_LOOP_DEPTH];
int loopEndStack[MAX_LOOP_DEPTH];
int loopDepth = 0;
int label=0;
int libMode = 0;
int nextfree=0;
int arrayErrorLabel=-1;
extern int nextBinding;
int codegen(struct tnode *t);
struct Gsymbol *codegenFunc = NULL;   /* set by codegenFunction() below */
static void libHeader(const char *name, const char *next){
    fprintf(fp, "MOV R1, SP\n");
    fprintf(fp, "MOV R2, 5\n");
    fprintf(fp, "SUB R1, R2\n");
    fprintf(fp, "MOV R2, \"%s\"\n", name);
    fprintf(fp, "MOV R1, [R1]\n");
    fprintf(fp, "EQ R1, R2\n");
    fprintf(fp, "JZ R1, %s\n", next);
}

static void libReturn(void){
    fprintf(fp, "MOV R2, SP\n");
    fprintf(fp, "MOV R3, 1\n");
    fprintf(fp, "SUB R2, R3\n");
    fprintf(fp, "MOV [R2], R1\n");
    fprintf(fp, "RET\n");
}

static void libIntBlock(const char *name, const char *next, int code, int intno){
    libHeader(name, next);
    fprintf(fp, "MOV R1, SP\n");
    fprintf(fp, "MOV R2, 4\n");
    fprintf(fp, "SUB R1, R2\n");
    fprintf(fp, "MOV R2, [R1]\n");
    fprintf(fp, "ADD R1, 1\n");
    fprintf(fp, "MOV R3, [R1]\n");
    fprintf(fp, "ADD R1, 1\n");
    fprintf(fp, "MOV R4, [R1]\n");
    fprintf(fp, "MOV R5, %d\n", code);
    fprintf(fp, "PUSH R5\n");
    fprintf(fp, "PUSH R2\n");
    fprintf(fp, "PUSH R3\n");
    fprintf(fp, "PUSH R4\n");
    fprintf(fp, "PUSH R5\n");
    fprintf(fp, "INT %d\n", intno);
    fprintf(fp, "POP R1\n");
    for(int i = 0; i < 4; i++) fprintf(fp, "POP R2\n");
    libReturn();
}

static void libCallBlock(const char *name, const char *next, int flabel, int hasArg){
    libHeader(name, next);
    if(hasArg){
        fprintf(fp, "MOV R1, SP\n");
        fprintf(fp, "MOV R2, 4\n");
        fprintf(fp, "SUB R1, R2\n");
        fprintf(fp, "MOV R1, [R1]\n");
        fprintf(fp, "PUSH R1\n");
    }
    fprintf(fp, "MOV R1, 0\n");
    fprintf(fp, "PUSH R1\n");
    fprintf(fp, "CALL F%d\n", flabel);
    fprintf(fp, "POP R1\n");
    if(hasArg) fprintf(fp, "SUB SP, 1\n");
    libReturn();
}

void writeLibraryDispatcher(void){
    struct Gsymbol *fi = Lookup("Initialize");
    struct Gsymbol *fa = Lookup("Alloc");
    struct Gsymbol *ff = Lookup("Free");
    if(fi == NULL || fa == NULL || ff == NULL || !fi->isFunction || !fa->isFunction || !ff->isFunction){
        printf("Error: library must define Initialize, Alloc and Free\n");
        exit(1);
    }
    libIntBlock("Write", "LIBREAD", 5, 7);
    fprintf(fp, "LIBREAD:\n");
    libIntBlock("Read", "LIBEXIT", 7, 6);
    fprintf(fp, "LIBEXIT:\n");
    libHeader("Exit", "LIBINIT");
    fprintf(fp, "MOV R5, 10\n");
    for(int i = 0; i < 5; i++) fprintf(fp, "PUSH R5\n");
    fprintf(fp, "INT 10\n");
    fprintf(fp, "RET\n");
    fprintf(fp, "LIBINIT:\n");
    libCallBlock("Heapset", "LIBALLOC", fi->flabel, 0);
    fprintf(fp, "LIBALLOC:\n");
    libCallBlock("Alloc", "LIBFREE", fa->flabel, 1);
    fprintf(fp, "LIBFREE:\n");
    libCallBlock("Free", "LIBEND", ff->flabel, 1);
    fprintf(fp, "LIBEND:\n");
    fprintf(fp, "RET\n");
}
int getlabel(){
    return label++;
}
int getreg(){
    int reg=nextfree;
    nextfree++;
    return reg;
}
void freereg(){
    nextfree--;
}
void writeHeader(){
    fprintf(fp, "0\n2056\n0\n0\n0\n0\n0\n1\n");
    int sp = (nextBinding > 4096) ? nextBinding - 1 : 4096;
    fprintf(fp, "MOV SP, %d\n", sp);
}
struct tnode *makeArgNode(struct tnode *expr){
    struct tnode *n = malloc(sizeof(struct tnode));
    n->nodetype = NODE_ARG;
    n->type = TYPE_NONE;
    n->val = 0;
    n->varname = NULL;
    n->Gentry = NULL;
    n->Lentry = NULL;
    n->left = expr;
    n->middle = NULL;
    n->right = NULL;
    n->ttype=NULL;
    n->field = NULL;
    return n;
}
struct tnode *appendArg(struct tnode *list, struct tnode *p){
    if(list == NULL) return p;
    struct tnode *q = list;
    while(q->right != NULL) q = q->right;
    q->right = p;
    return list;
}
int codegenScalarAddr(struct tnode *idnode){
    int r = getreg();
    if(idnode->Lentry != NULL){
        fprintf(fp, "MOV R%d, BP\n", r);
        if(idnode->Lentry->binding < 0)
            fprintf(fp, "SUB R%d, %d\n", r, -idnode->Lentry->binding);
        else
            fprintf(fp, "ADD R%d, %d\n", r, idnode->Lentry->binding);
    } else {
        fprintf(fp, "MOV R%d, %d\n", r, idnode->Gentry->binding);
    }
    return r;
}
int codegenaddr2d(struct tnode *t){
    int r=getreg();
    fprintf(fp,"MOV R%d, %d\n",r,t->left->Gentry->binding);

    int rrow=codegen(t->middle);
    int rows = t->left->Gentry->size / t->left->Gentry->cols;

    if(t->middle->nodetype != 'N' && !libMode){
        if(arrayErrorLabel == -1) arrayErrorLabel = getlabel();

        int rlo = getreg();
        fprintf(fp, "MOV R%d, R%d\n", rlo, rrow);
        int rzero = getreg();
        fprintf(fp, "MOV R%d, 0\n", rzero);
        fprintf(fp, "LT R%d, R%d\n", rlo, rzero);
        fprintf(fp, "JNZ R%d, L%d\n", rlo, arrayErrorLabel);
        freereg(); freereg();

        int rhi = getreg();
        fprintf(fp, "MOV R%d, R%d\n", rhi, rrow);
        int rrows = getreg();
        fprintf(fp, "MOV R%d, %d\n", rrows, rows);
        fprintf(fp, "GE R%d, R%d\n", rhi, rrows);
        fprintf(fp, "JNZ R%d, L%d\n", rhi, arrayErrorLabel);
        freereg(); freereg();
    }

    int rcol=codegen(t->right);

    if(t->right->nodetype != 'N' && !libMode){
        if(arrayErrorLabel == -1) arrayErrorLabel = getlabel();

        int rlo = getreg();
        fprintf(fp, "MOV R%d, R%d\n", rlo, rcol);
        int rzero = getreg();
        fprintf(fp, "MOV R%d, 0\n", rzero);
        fprintf(fp, "LT R%d, R%d\n", rlo, rzero);
        fprintf(fp, "JNZ R%d, L%d\n", rlo, arrayErrorLabel);
        freereg(); freereg();

        int rhi = getreg();
        fprintf(fp, "MOV R%d, R%d\n", rhi, rcol);
        int rcols = getreg();
        fprintf(fp, "MOV R%d, %d\n", rcols, t->left->Gentry->cols);
        fprintf(fp, "GE R%d, R%d\n", rhi, rcols);
        fprintf(fp, "JNZ R%d, L%d\n", rhi, arrayErrorLabel);
        freereg(); freereg();
    }

    int rcolsize=getreg();
    fprintf(fp, "MOV R%d, %d\n",rcolsize,t->left->Gentry->cols);
    fprintf(fp, "MUL R%d, R%d\n",rrow,rcolsize);
    freereg();

    fprintf(fp, "ADD R%d, R%d\n",rrow,rcol);
    freereg();

    fprintf(fp, "ADD R%d, R%d\n",r,rrow);
    freereg();
    return r;
}
int codegenaddr(struct tnode *t){
    if(t->nodetype=='V'){
        int r=getreg();
        fprintf(fp, "MOV R%d, %d\n", r, t->Gentry->binding);
        return r;
    }
    if(t->nodetype==NODE_ARRAY){
        int r=getreg();
        fprintf(fp, "MOV R%d, %d\n", r, t->left->Gentry->binding);

        int rindex = codegen(t->right);

        if(t->right->nodetype != 'N'){
            if(arrayErrorLabel == -1) arrayErrorLabel = getlabel();

            int rlo = getreg();
            fprintf(fp, "MOV R%d, R%d\n", rlo, rindex);
            int rzero = getreg();
            fprintf(fp, "MOV R%d, 0\n", rzero);
            fprintf(fp, "LT R%d, R%d\n", rlo, rzero);
            fprintf(fp, "JNZ R%d, L%d\n", rlo, arrayErrorLabel);
            freereg();
            freereg();

            int rhi = getreg();
            fprintf(fp, "MOV R%d, R%d\n", rhi, rindex);
            int rsize = getreg();
            fprintf(fp, "MOV R%d, %d\n", rsize, t->left->Gentry->size);
            fprintf(fp, "GE R%d, R%d\n", rhi, rsize);
            fprintf(fp, "JNZ R%d, L%d\n", rhi, arrayErrorLabel);
            freereg();
            freereg();
        }

        fprintf(fp, "ADD R%d, R%d\n", r, rindex);
        freereg();
        return r;
    }
    return -1;
}
void writeArrayErrorHandler(){
    if(arrayErrorLabel == -1) return;
    fprintf(fp, "L%d:\n", arrayErrorLabel);
    fprintf(fp, "MOV R0, 0\n");
    fprintf(fp, "MOV R1, \"Exit\"\n");
    fprintf(fp, "PUSH R1\n");
    fprintf(fp, "PUSH R0\n");
    fprintf(fp, "PUSH R0\n");
    fprintf(fp, "PUSH R0\n");
    fprintf(fp, "PUSH R0\n");
    fprintf(fp, "CALL 0\n");
}
int codegenFieldAddr(struct tnode *t){
    struct tnode *v = t->left;
    if(t->field != NULL){
        int r = codegen(v);
        if(t->field->fieldIndex > 0) fprintf(fp, "ADD R%d, %d\n", r, t->field->fieldIndex);
        return r;
    }
    int isptr = (v->Lentry != NULL) ? v->Lentry->isPointer : v->Gentry->isPointer;
    int r = isptr ? codegen(v) : codegenScalarAddr(v);
    if(t->val > 0) fprintf(fp, "ADD R%d, %d\n", r, t->val);
    return r;
}
int codegenTupleAddr(struct tnode *t){
    if(t->nodetype == NODE_DEREF) return codegen(t->left);
    if(t->nodetype == 'V') return codegenScalarAddr(t);
    printf("Error: here a tuple must be a tuple variable or *pointer\n");
    exit(1);
}
void codegenCopyWords(int rl, int rr, int size){
    int rt = getreg();
    for(int i = 0; i < size; i++){
        fprintf(fp, "MOV R%d, [R%d]\n", rt, rr);
        fprintf(fp, "MOV [R%d], R%d\n", rl, rt);
        if(i < size - 1){
            fprintf(fp, "ADD R%d, 1\n", rl);
            fprintf(fp, "ADD R%d, 1\n", rr);
        }
    }
    freereg();
}
void codegenTupleCopy(struct tnode *t){
    int size = t->left->ttype->size;
    int rl = codegenTupleAddr(t->left);
    int rr = codegenTupleAddr(t->right);
    codegenCopyWords(rl, rr, size);
    freereg(); freereg();
}
void codegenStmt(struct tnode *t){
    codegen(t);
    if(t != NULL && (t->nodetype == NODE_CALL || t->nodetype == NODE_INIT || t->nodetype == NODE_FREE)) freereg();
}

static int libCall(const char *fn, struct tnode *arg, int cnst){
    int ra = arg ? codegen(arg) : getreg();
    for(int i = 0; i < ra; i++) fprintf(fp, "PUSH R%d\n", i);
    int rs = getreg();
    fprintf(fp, "MOV R%d, \"%s\"\n", rs, fn);
    fprintf(fp, "PUSH R%d\n", rs);
    if(!arg) fprintf(fp, "MOV R%d, %d\n", ra, cnst);
    fprintf(fp, "PUSH R%d\n", ra);
    fprintf(fp, "MOV R%d, 0\n", rs);
    fprintf(fp, "PUSH R%d\n", rs);
    fprintf(fp, "PUSH R%d\n", rs);
    fprintf(fp, "PUSH R%d\n", rs);
    fprintf(fp, "CALL 0\n");
    fprintf(fp, "POP R%d\n", ra);
    for(int k = 0; k < 4; k++) fprintf(fp, "POP R%d\n", rs);
    freereg();
    for(int i = ra - 1; i >= 0; i--) fprintf(fp, "POP R%d\n", i);
    return ra;
}
int exprIsPointer(struct tnode *e){
    if(e->nodetype == NODE_ADDR) return 1;
    if(e->nodetype == NODE_DEREF) return 0;
    if(e->nodetype == 'V'){
        if(e->Lentry != NULL) return e->Lentry->isPointer;
        if(e->Gentry != NULL) return e->Gentry->isPointer;
    }
    if(e->nodetype == NODE_CALL) return e->Gentry->isPointer;
    return 0;
}
static int argWordsOf(struct tnode *call){
    int w = 0;
    for(struct tnode *a = call->right; a != NULL; a = a->right){
        struct tnode *e = a->left;
        w += (e->type == TYPE_TUPLE && !exprIsPointer(e)) ? e->ttype->size : 1;
    }
    return w;
}
static void pushArgs(struct tnode *call){
    int n = 0;
    for(struct tnode *a = call->right; a != NULL; a = a->right) n++;
    struct tnode **args = malloc((n ? n : 1) * sizeof(*args));
    struct tnode *a = call->right;
    for(int i = 0; i < n; i++){ args[i] = a->left; a = a->right; }
    for(int i = n - 1; i >= 0; i--){
        struct tnode *e = args[i];
        if(e->type == TYPE_TUPLE && !exprIsPointer(e)){
            int s = e->ttype->size;
            int ra = codegenTupleAddr(e);
            int rt = getreg();
            for(int k = 0; k < s; k++){
                fprintf(fp, "MOV R%d, [R%d]\n", rt, ra);
                fprintf(fp, "PUSH R%d\n", rt);
                if(k < s - 1) fprintf(fp, "ADD R%d, 1\n", ra);
            }
            freereg(); freereg();
        } else {
            int r = codegen(e);
            fprintf(fp, "PUSH R%d\n", r);
            freereg();
        }
    }
    free(args);
}
void codegenTupleCallAssign(struct tnode *t){     /* t is '=' with a by-value tuple call on the right */
    struct tnode *call = t->right;
    struct Gsymbol *f = call->Gentry;
    int s = f->ttype->size;
    int saved = nextfree;
    for(int i = 0; i < saved; i++) fprintf(fp, "PUSH R%d\n", i);
    pushArgs(call);
    int rz = getreg();
    fprintf(fp, "MOV R%d, 0\n", rz);
    for(int k = 0; k < s; k++) fprintf(fp, "PUSH R%d\n", rz);   /* reserve the return area */
    freereg();
    fprintf(fp, "CALL F%d\n", f->flabel);
    int rd = codegenTupleAddr(t->left);               /* destination, after the call */
    int rb = getreg();
    fprintf(fp, "MOV R%d, SP\n", rb);                 /* SP points at the last word of the area */
    if(s > 1) fprintf(fp, "SUB R%d, %d\n", rb, s - 1);
    codegenCopyWords(rd, rb, s);
    freereg(); freereg();
    fprintf(fp, "SUB SP, %d\n", s + argWordsOf(call));
    for(int i = saved - 1; i >= 0; i--) fprintf(fp, "POP R%d\n", i);
}
struct tnode* createtree(int val, struct Typetable *type, int nodetype, char* varname, struct tnode* l, struct tnode *m, struct tnode* r)
{
    struct tnode* temp = malloc(sizeof(struct tnode));
    temp->val=val;
    temp->type=type;
    temp->nodetype=nodetype;
    if(varname!=NULL)temp->varname=strdup(varname);
    else temp->varname=NULL;
    temp->left=l;
    temp->middle=m;
    temp->right=r;

    temp->Gentry=NULL;
    temp->ttype=NULL;
    temp->Lentry=NULL;
    temp->field = NULL;

    if(nodetype=='V'){
        struct Lsymbol *lentry=LLookup(varname);
        if(lentry!=NULL){
            temp->Lentry=lentry;
            temp->type=lentry->type;
            temp->ttype=lentry->ttype;
        }
        else {
            struct Gsymbol *gentry=Lookup(varname);
            if(gentry==NULL){
                printf("Error: Variable %s not declared\n",varname);
                exit(1);
            }
            struct Gsymbol *cf = GetCurrentFunction();
            if(cf != NULL && cf->owner != NULL){
                printf("Error: method %s cannot access global variable %s\n", cf->name, varname);
                exit(1);
            }
            temp->Gentry=gentry;
            temp->type=gentry->type;
            temp->ttype=gentry->ttype;
        }
        
    }
    else if(nodetype=='+' || nodetype=='*'|| nodetype == '-' || nodetype == '/'){ //arithmatic
        if(l->type!=TYPE_INT || r->type !=TYPE_INT){
            printf("Type error: invalid operands\n");
            exit(1);
        }
        temp->type=TYPE_INT;
    }
    else if(nodetype=='<' || nodetype=='>' || nodetype==NODE_LE || nodetype==NODE_GE || nodetype==NODE_EQ || nodetype==NODE_NE){
        int ok;
        if(nodetype==NODE_EQ || nodetype==NODE_NE){
            if(isUserType(l->type) || isUserType(r->type))
                ok = userEqOK(l->type, r->type);
            else
                ok = (l->type==TYPE_INT && r->type==TYPE_INT) || (l->type==TYPE_STR && r->type==TYPE_STR);
        } else
            ok = (l->type==TYPE_INT && r->type==TYPE_INT);
        if(!ok){ printf("Type error in comparison\n"); exit(1); }
        temp->type=TYPE_BOOL;
    }
    else if(nodetype=='='){
        if(!typeCompatible(l->type, r->type) || exprIsPointer(l)!=exprIsPointer(r) || l->ttype!=r->ttype){
            printf("Type mismatch in assignment\n");
            exit(1);
        }
        temp->type=TYPE_NONE;
    }
     else if(nodetype == 'I'){
        if(l->type != TYPE_BOOL){
            printf("Type error: IF condition must be boolean\n");
            exit(1);
        }
        temp->type = TYPE_NONE;
    }

    else if(nodetype == 'L' || nodetype=='U' || nodetype=='D'){
        if(l->type != TYPE_BOOL){
            printf("Type error: WHILE condition must be boolean\n");
            exit(1);
        }
        temp->type = TYPE_NONE;
    }

    else if(nodetype=='R' || nodetype=='W' || nodetype=='C'){
        if((nodetype=='R' || nodetype=='W') && l->type==TYPE_TUPLE && !exprIsPointer(l)){
            printf("Error: cannot read or write a whole tuple\n");
            exit(1);
        }
        if((nodetype=='R' || nodetype=='W') && (isUserType(l->type) || l->type==TVOID)){
            printf("Error: cannot read or write a user-defined type or NULL\n");
            exit(1);
        }
        temp->type=TYPE_NONE;
    }
    else if(nodetype == NODE_ARRAY){
        if(r->type != TYPE_INT){
            printf("Type error: array index must be integer\n");
            exit(1);
        }

        temp->type = l->type;
        temp->Gentry = l->Gentry;
    }
    else if(nodetype==NODE_ARRAY2D){
        if(m->type!=TYPE_INT || r->type!=TYPE_INT){
            printf("Array indices must be integers\n");
            exit(1);
        }
        temp->type=l->type;
        temp->Gentry=l->Gentry;
    }
    else if(nodetype==NODE_ADDR){
        temp->type = (l->Lentry != NULL) ? l->Lentry->type : l->Gentry->type;
        temp->Gentry = l->Gentry;
        temp->Lentry = l->Lentry;
        temp->ttype=l->ttype;
    }
    else if(nodetype==NODE_DEREF){
        temp->type = (l->Lentry != NULL) ? l->Lentry->type : l->Gentry->type;
        temp->Gentry = l->Gentry;
        temp->Lentry = l->Lentry;
        temp->ttype=l->ttype;
    }
    else if(nodetype == NODE_CALL){
        struct Gsymbol *cf0 = GetCurrentFunction();
        if(cf0 != NULL && cf0->owner != NULL){
            printf("Error: method %s cannot call global function %s\n", cf0->name, varname);
            exit(1);
        }
        struct Gsymbol *fentry = Lookup(varname);
        if(fentry == NULL || !fentry->isFunction){
            printf("Error: %s is not a function\n", varname);
            exit(1);
        }
        struct Paramstruct *p = fentry->paramlist;
        struct tnode *a = r;
        while(p != NULL && a != NULL){
            if(!typeCompatible(p->type, a->left->type) || exprIsPointer(a->left) != p->isPointer || a->left->ttype != p->ttype){
                printf("Error: argument type mismatch in call to %s\n", varname);
                exit(1);
            }
            p = p->next;
            a = a->right;
        }
        if(p != NULL || a != NULL){
            printf("Error: argument count mismatch in call to %s\n", varname);
            exit(1);
        }
        temp->Gentry = fentry;
        temp->type = fentry->type;
        temp->ttype = fentry->ttype;
    }
    else if(nodetype == NODE_RETURN){
        struct Gsymbol *cf = GetCurrentFunction();
        if(cf == NULL){
            printf("Error: return outside a function\n");
            exit(1);
        }
        if(!typeCompatible(cf->type, l->type) || exprIsPointer(l) != cf->isPointer || l->ttype != cf->ttype){
            printf("Error: return type mismatch in function %s\n", cf->name);
            exit(1);
        }
        temp->type = TYPE_NONE;
    }
    return temp;
}
struct tnode *makeTupleFieldNode(char *var, char *field){
    struct Lsymbol *le = NULL; struct Gsymbol *ge = NULL;
    int found = LookupVar(var, &le, &ge);
    if(found == 0){ printf("Variable %s not declared\n", var); exit(1); }
    struct TupleType *tt = (found == 1) ? le->ttype : ge->ttype;
    if(tt == NULL){ printf("Error: %s is not a tuple\n", var); exit(1); }
    struct Field *f = FindField(tt, field);
    if(f == NULL){ printf("Error: tuple %s has no field %s\n", tt->name, field); exit(1); }
    struct tnode *idnode = createtree(0, TYPE_NONE, 'V', var, NULL, NULL, NULL);
    return createtree(f->pos, f->type, NODE_FIELD, NULL, idnode, NULL, NULL);
}
int codegen(struct tnode *t){

    if(t == NULL)
        return -1;

    switch(t->nodetype){
        case 'N': {
            int r = getreg();
            fprintf(fp, "MOV R%d, %d\n", r, t->val);
            return r;
        }
        case 'V': {
            if(t->Lentry!=NULL){
                int addr=codegenScalarAddr(t);
                fprintf(fp, "MOV R%d, [R%d]\n",addr,addr);
                return addr;
            }
            int r=getreg();
            fprintf(fp, "MOV R%d, [%d]\n", r, t->Gentry->binding);
            return r;
        }

        case '=': {
            if(t->left->type == TYPE_TUPLE && !exprIsPointer(t->left)){
                if(t->right->nodetype == NODE_CALL) codegenTupleCallAssign(t);
                else codegenTupleCopy(t);
                return -1;
            }
            int r = codegen(t->right);

            if(t->left->nodetype == NODE_ARRAY && t->left->right->nodetype == 'N'){
                fprintf(fp, "MOV [%d], R%d\n", t->left->left->Gentry->binding + t->left->right->val, r);
                freereg();
            }
            else if(t->left->nodetype == NODE_ARRAY)
            {
                int addr = codegenaddr(t->left);

                fprintf(fp, "MOV [R%d], R%d\n", addr, r);

                freereg();   // address register
                freereg();   // value register
            }
            else if(t->left->nodetype==NODE_ARRAY2D){
                int addr=codegenaddr2d(t->left);
                fprintf(fp, "MOV [R%d], R%d\n",addr,r);
                freereg();
                freereg();
            }
            else if(t->left->nodetype==NODE_DEREF){
                int addr;
                if(t->left->left->Lentry != NULL){
                    addr = codegenScalarAddr(t->left->left);
                    fprintf(fp, "MOV R%d, [R%d]\n", addr, addr);
                } else {
                    addr=getreg();
                    fprintf(fp, "MOV R%d, [%d]\n", addr, t->left->left->Gentry->binding);
                }
                fprintf(fp, "MOV [R%d], R%d\n", addr, r);
                freereg();
                freereg();
            }
              else if(t->left->nodetype == NODE_FIELD){
                int addr = codegenFieldAddr(t->left);
                fprintf(fp, "MOV [R%d], R%d\n", addr, r);
                freereg(); freereg();
            }
            else if(t->left->Lentry != NULL){
                int addr = codegenScalarAddr(t->left);
                fprintf(fp, "MOV [R%d], R%d\n", addr, r);
                freereg(); freereg();
            }
            else
            {
                fprintf(fp, "MOV [%d], R%d\n", t->left->Gentry->binding, r);
                freereg();
            }

            return -1;
        }

        case 'C': {
            codegenStmt(t->left);
            codegenStmt(t->right);
            return -1;
        }

        case '+':{
            int r1=codegen(t->left);
            int r2=codegen(t->right);
            fprintf(fp,"ADD R%d, R%d\n",r1,r2);
            freereg();
            return r1;
        }
        case '-':{
            int r1=codegen(t->left);
            int r2=codegen(t->right);
            fprintf(fp,"SUB R%d, R%d\n",r1,r2);
            freereg();
            return r1;
        }
        case '*':{
            int r1=codegen(t->left);
            int r2=codegen(t->right);
            fprintf(fp,"MUL R%d, R%d\n",r1,r2);
            freereg();
            return r1;
        }
        case '/':{
            int r1=codegen(t->left);
            int r2=codegen(t->right);
            fprintf(fp,"DIV R%d, R%d\n",r1,r2);
            freereg();
            return r1;
        }
        case '<': {
            int r1=codegen(t->left);
            int r2=codegen(t->right);
            fprintf(fp,"LT R%d, R%d\n",r1,r2);
            freereg();
            return r1;
        }
        case '>': {
            int r1=codegen(t->left);
            int r2=codegen(t->right);
            fprintf(fp,"GT R%d, R%d\n",r1,r2);
            freereg();
            return r1;
        }
        case NODE_LE: {
            int r1=codegen(t->left);
            int r2=codegen(t->right);
            fprintf(fp,"LE R%d, R%d\n",r1,r2);
            freereg();
            return r1;
        }
        case NODE_GE: {
            int r1=codegen(t->left);
            int r2=codegen(t->right);
            fprintf(fp,"GE R%d, R%d\n",r1,r2);
            freereg();
            return r1;
        }
        case NODE_EQ: {
            int r1=codegen(t->left);
            int r2=codegen(t->right);
            fprintf(fp,"EQ R%d, R%d\n",r1,r2);
            freereg();
            return r1;
        }
        case NODE_NE: {
            int r1=codegen(t->left);
            int r2=codegen(t->right);
            fprintf(fp,"NE R%d, R%d\n",r1,r2);
            freereg();
            return r1;
        }
        case 'L':{
            int label1=getlabel();
            int label2=getlabel();
            fprintf(fp, "L%d:\n",label1);
            int r=codegen(t->left);
            fprintf(fp, "JZ R%d, L%d\n",r,label2);
            freereg();

            loopStartStack[loopDepth] = label1;
            loopEndStack[loopDepth] = label2;
            loopDepth++;

            codegenStmt(t->middle);

            loopDepth--;

            fprintf(fp,"JMP L%d\n",label1);
            fprintf(fp,"L%d:\n",label2);
            return -1;
        }
        case 'I':{
            int r=codegen(t->left);
            if(t->right==NULL){
                //if-then
                int label=getlabel();
                fprintf(fp,"JZ R%d, L%d\n",r,label);
                freereg();
                codegenStmt(t->middle);
                fprintf(fp,"L%d:\n",label);
            }
            else{
                //if-then-else
                int elselabel=getlabel();
                int endlabel=getlabel();
                fprintf(fp,"JZ R%d, L%d\n",r,elselabel);
                freereg();
                //then
                codegenStmt(t->middle);
                fprintf(fp,"JMP L%d\n",endlabel);
                //else
                fprintf(fp,"L%d:\n",elselabel);
                codegenStmt(t->right);
                fprintf(fp,"L%d:\n",endlabel);
            }
            return -1;
        }
       case 'R':
        {
            int addr;
            
            if(t->left->nodetype=='V'){
                if(t->left->Lentry != NULL){
                    int addr = codegenScalarAddr(t->left);
                    fprintf(fp, "MOV R2, \"Read\"\n"); fprintf(fp, "PUSH R2\n");
                    fprintf(fp, "MOV R2, -1\n"); fprintf(fp, "PUSH R2\n");
                    fprintf(fp, "PUSH R%d\n", addr);
                    fprintf(fp, "PUSH R0\n"); fprintf(fp, "PUSH R0\n");
                    fprintf(fp, "CALL 0\n");
                    fprintf(fp, "POP R0\n"); fprintf(fp, "POP R0\n"); fprintf(fp, "POP R0\n"); fprintf(fp, "POP R0\n"); fprintf(fp, "POP R0\n");
                    freereg();
                } 
                else
                {
                    addr=t->left->Gentry->binding;
                    fprintf(fp, "MOV R2, \"Read\"\n");
                    fprintf(fp, "PUSH R2\n");
                    fprintf(fp, "MOV R2, -1\n");
                    fprintf(fp, "PUSH R2\n");
                    fprintf(fp, "MOV R2, %d\n", addr);
                    fprintf(fp, "PUSH R2\n");
                    fprintf(fp, "PUSH R0\n");
                    fprintf(fp, "PUSH R0\n");
                    fprintf(fp, "CALL 0\n");
                    fprintf(fp, "POP R0\n");
                    fprintf(fp, "POP R0\n");
                    fprintf(fp, "POP R0\n");
                    fprintf(fp, "POP R0\n");
                    fprintf(fp, "POP R0\n");
                }

        }
        else if(t->left->nodetype==NODE_ARRAY){
            int addr = codegenaddr(t->left);

            fprintf(fp, "MOV R2, \"Read\"\n");
            fprintf(fp, "PUSH R2\n");
            fprintf(fp, "MOV R2, -1\n");
            fprintf(fp, "PUSH R2\n");
            fprintf(fp, "PUSH R%d\n", addr);
            fprintf(fp, "PUSH R0\n");
            fprintf(fp, "PUSH R0\n");
            fprintf(fp, "CALL 0\n");
            fprintf(fp, "POP R0\n");
            fprintf(fp, "POP R0\n");
            fprintf(fp, "POP R0\n");
            fprintf(fp, "POP R0\n");
            fprintf(fp, "POP R0\n");

            freereg();
        }
        else if(t->left->nodetype==NODE_DEREF){
            int addr = codegen(t->left->left);   /* value of p = address to read into */
            fprintf(fp, "MOV R2, \"Read\"\n");
            fprintf(fp, "PUSH R2\n");
            fprintf(fp, "MOV R2, -1\n");
            fprintf(fp, "PUSH R2\n");
            fprintf(fp, "PUSH R%d\n", addr);
            fprintf(fp, "PUSH R0\n");
            fprintf(fp, "PUSH R0\n");
            fprintf(fp, "CALL 0\n");
            fprintf(fp, "POP R0\n");
            fprintf(fp, "POP R0\n");
            fprintf(fp, "POP R0\n");
            fprintf(fp, "POP R0\n");
            fprintf(fp, "POP R0\n");

            freereg();
        }
        else if(t->left->nodetype==NODE_FIELD){
            int addr = codegenFieldAddr(t->left);
            fprintf(fp, "MOV R2, \"Read\"\n");
            fprintf(fp, "PUSH R2\n");
            fprintf(fp, "MOV R2, -1\n");
            fprintf(fp, "PUSH R2\n");
            fprintf(fp, "PUSH R%d\n", addr);
            fprintf(fp, "PUSH R0\n");
            fprintf(fp, "PUSH R0\n");
            fprintf(fp, "CALL 0\n");
            fprintf(fp, "POP R0\n");
            fprintf(fp, "POP R0\n");
            fprintf(fp, "POP R0\n");
            fprintf(fp, "POP R0\n");
            fprintf(fp, "POP R0\n");
            freereg();
        }
            return -1;
        }

        /* WRITE */
        case 'W':
        {
            int r = codegen(t->left);

            fprintf(fp, "MOV R2, \"Write\"\n");
            fprintf(fp, "PUSH R2\n");

            fprintf(fp, "MOV R2, -2\n");
            fprintf(fp, "PUSH R2\n");

            fprintf(fp, "PUSH R%d\n", r);

            fprintf(fp, "PUSH R0\n");
            fprintf(fp, "PUSH R0\n");

            fprintf(fp, "CALL 0\n");

            fprintf(fp, "POP R0\n");
            fprintf(fp, "POP R0\n");
            fprintf(fp, "POP R0\n");
            fprintf(fp, "POP R0\n");
            fprintf(fp, "POP R0\n");

            freereg();

            return -1;
        }
        case 'B':{
            if(loopDepth==0)return -1;
            fprintf(fp, "JMP L%d\n",loopEndStack[loopDepth-1]);
            return -1;
        }
        case 'K':{
            if(loopDepth==0)return -1;
            fprintf(fp, "JMP L%d\n",loopStartStack[loopDepth-1]);
            return -1;
        }
        case 'D':{
            int bodylabel=getlabel();
            int condlabel=getlabel();
            int endlabel=getlabel();

            fprintf(fp,"L%d:\n",bodylabel);

            loopStartStack[loopDepth]=condlabel;
            loopEndStack[loopDepth]=endlabel;
            loopDepth++;

            codegenStmt(t->middle);

            loopDepth--;

            fprintf(fp,"L%d:\n",condlabel);
            int r=codegen(t->left);
            fprintf(fp, "JNZ R%d, L%d\n",r,bodylabel);
            freereg();

            fprintf(fp,"L%d:\n",endlabel);
            return -1;
        }
        case 'U':{
            int bodylabel=getlabel();
            int condlabel=getlabel();
            int endlabel=getlabel();

            fprintf(fp,"L%d:\n",bodylabel);

            loopStartStack[loopDepth]=condlabel;
            loopEndStack[loopDepth]=endlabel;
            loopDepth++;

            codegenStmt(t->middle);

            loopDepth--;

            fprintf(fp,"L%d:\n",condlabel);
            int r=codegen(t->left);
            fprintf(fp, "JZ R%d, L%d\n",r,bodylabel);
            freereg();

            fprintf(fp,"L%d:\n",endlabel);
            return -1;
        }
        case NODE_ARRAY:
        {
            if(t->right->nodetype == 'N'){
                int rc = getreg();
                fprintf(fp, "MOV R%d, [%d]\n", rc, t->left->Gentry->binding + t->right->val);
                return rc;
            }
            int addr = codegenaddr(t);
            fprintf(fp, "MOV R%d, [R%d]\n", addr, addr);
            return addr;
        }
        case NODE_ARRAY2D:
        {
            int addr=codegenaddr2d(t);
            fprintf(fp, "MOV R%d, [R%d]\n", addr, addr);
            return addr;
        }
        case NODE_FIELD:
        {
            int r = codegenFieldAddr(t);
            fprintf(fp, "MOV R%d, [R%d]\n", r, r);
            return r;
        }
        case NODE_STRCONST:
        {
            int r = getreg();
            fprintf(fp, "MOV R%d, \"%s\"\n", r, t->varname);
            return r;
        }
        case NODE_ADDR: {
            if(t->left->Lentry != NULL) return codegenScalarAddr(t->left);
            int r=getreg();
            fprintf(fp, "MOV R%d, %d\n",r,t->left->Gentry->binding);
            return r;
        }
        case NODE_DEREF: {
            int r;
            if(t->left->Lentry != NULL){
                r = codegenScalarAddr(t->left);
                fprintf(fp, "MOV R%d, [R%d]\n", r, r);
            } else {
                r = getreg();
                fprintf(fp, "MOV R%d, [%d]\n", r, t->left->Gentry->binding);
            }
            fprintf(fp, "MOV R%d, [R%d]\n", r, r);
            return r;
        }
        case NODE_CALL: {
            struct Gsymbol *f = t->Gentry;
            if(f->type == TYPE_TUPLE && !f->isPointer){
                printf("Error: result of tuple-returning function %s must be assigned to a tuple variable\n", f->name);
                exit(1);
            }
            int saved = nextfree;
            for(int i = 0; i < saved; i++) fprintf(fp, "PUSH R%d\n", i);
            pushArgs(t);
            int rv = getreg();
            fprintf(fp, "MOV R%d, 0\n", rv);
            fprintf(fp, "PUSH R%d\n", rv);
            freereg();
            fprintf(fp, "CALL F%d\n", f->flabel);
            int result = getreg();
            fprintf(fp, "POP R%d\n", result);
            int aw = argWordsOf(t);
            if(aw > 0) fprintf(fp, "SUB SP, %d\n", aw);
            for(int i = saved - 1; i >= 0; i--) fprintf(fp, "POP R%d\n", i);
            return result;
        }

        case NODE_RETURN: {
            if(strcmp(codegenFunc->name, "main") == 0){
                codegen(t->left);
                fprintf(fp, "HALT\n");
                return -1;
            }
            if(codegenFunc->type == TYPE_TUPLE && !codegenFunc->isPointer){
                int s = codegenFunc->ttype->size;
                int rl = getreg();
                fprintf(fp, "MOV R%d, BP\n", rl);
                fprintf(fp, "SUB R%d, %d\n", rl, 1 + s);      /* base of the return area */
                int rr = codegenTupleAddr(t->left);
                codegenCopyWords(rl, rr, s);
                freereg(); freereg();
            } else {
                int r = codegen(t->left);
                int ra = getreg();
                fprintf(fp, "MOV R%d, BP\n", ra);
                fprintf(fp, "SUB R%d, 2\n", ra);
                fprintf(fp, "MOV [R%d], R%d\n", ra, r);
                freereg(); freereg();
            }
            if(codegenFunc->numLocals > 0)
                fprintf(fp, "SUB SP, %d\n", codegenFunc->numLocals);
            fprintf(fp, "POP BP\n");
            fprintf(fp, "RET\n");
            return -1;
        }
                case NODE_NULL: {
            int r = getreg();
            fprintf(fp, "MOV R%d, 0\n", r);
            return r;
        }
        case NODE_INIT:
            return libCall("Heapset", NULL, 0);
        case NODE_FREE:
            return libCall("Free", t->left, 0);
        case NODE_ALLOC: {
            int r = libCall("Alloc", NULL, t->left->type->size);
            if(t->left->nodetype == NODE_FIELD){
                int addr = codegenFieldAddr(t->left);
                fprintf(fp, "MOV [R%d], R%d\n", addr, r);
                freereg(); freereg();
            } else if(t->left->nodetype == NODE_ARRAY){
                int addr = codegenaddr(t->left);
                fprintf(fp, "MOV [R%d], R%d\n", addr, r);
                freereg(); freereg(); 
            } else if(t->left->Lentry != NULL){
                int addr = codegenScalarAddr(t->left);
                fprintf(fp, "MOV [R%d], R%d\n", addr, r);
                freereg(); freereg();
            } else {
                fprintf(fp, "MOV [%d], R%d\n", t->left->Gentry->binding, r);
                freereg();
            }
            return -1;
        }
    
        
      }

     return -1;
    }
void codegenFunction(struct Gsymbol *f){
    nextfree = 0;
    codegenFunc = f;
    if(strcmp(f->name,"main") != 0){
        fprintf(fp, "F%d:\n", f->flabel);
        fprintf(fp, "PUSH BP\n");
        fprintf(fp, "MOV BP, SP\n");
    } else {
        fprintf(fp, "F%d:\n", f->flabel);
        fprintf(fp, "MOV BP, SP\n");
    }
    if(f->numLocals > 0) fprintf(fp, "ADD SP, %d\n", f->numLocals);
    codegen(f->funcbody);
    codegenFunc = NULL;
}

static struct tnode *newNode(int nodetype, struct Typetable *type) {
    struct tnode *n = calloc(1, sizeof *n);      /* zeroes every pointer, including n->field */
    n->nodetype = nodetype;
    n->type = type;
    return n;
}

static void typeErr(const char *msg, const char *a, const char *b) {
    printf("Error: "); printf(msg, a, b); printf("\n");
    exit(1);
}

struct tnode *makeNullNode(void) { return newNode(NODE_NULL, TVOID); }
struct tnode *makeInitNode(void) { return newNode(NODE_INIT, TYPE_INT); }

struct tnode *makeAllocNode(struct tnode *var) {
    if (!isUserType(var->type) || var->type->isClass) typeErr("alloc() needs a variable of a user-defined type", "", "");
    struct tnode *n = newNode(NODE_ALLOC, var->type);
    n->left = var;
    return n;
}

struct tnode *makeFreeNode(struct tnode *var) {
    if(!isUserType(var->type) || var->type->isClass)typeErr("free() needs a variable of a user-defined type", "", "");
    struct tnode *n = newNode(NODE_FREE, TYPE_INT);
    n->left = var;
    return n;
}

struct tnode *makeFieldNode(struct tnode *base, char *fname) {
    if (base->type == TYPE_TUPLE)
        return makeTupleFieldNode(base->varname, fname);
    if (!isUserType(base->type))
        typeErr("cannot access field '%s': not a user-defined type", fname, "");
    if (base->type->isClass) {
        int isSelf = (base->nodetype == 'V' && base->varname && strcmp(base->varname, "self") == 0);
        if (!isSelf)
            typeErr("fields of class '%s' are private; cannot access '%s' outside its methods", base->type->name, fname);
    }
    Fieldlist *f = FLookup(base->type, fname);
    if (!f)
        typeErr("type '%s' has no field '%s'", base->type->name, fname);
    struct tnode *n = newNode(NODE_FIELD, f->type);
    n->left = base;
    n->field = f;
    n->varname = strdup(fname);
    return n;
}

int userEqOK(struct Typetable *a, struct Typetable *b) {
    return a == b || (isUserType(a) && b == TVOID) || (a == TVOID && isUserType(b));
}
struct tnode *makeSelfNode(void){
    struct Gsymbol *cf = GetCurrentFunction();
    if(cf == NULL || cf->owner == NULL){ printf("Error: self used outside a method\n"); exit(1); }
    return createtree(0, TYPE_NONE, 'V', "self", NULL, NULL, NULL);   /* resolves to the local */
}

struct tnode *makeMethodCallNode(struct tnode *obj, char *mname, struct tnode *args){
    if(!isClassType(obj->type)) typeErr("cannot call method '%s' on a non-object", mname, "");
    struct Classtable *c = obj->type->cls;
    struct Memberfunclist *m = Class_Mlookup(c, mname);
    if(m == NULL) typeErr("class '%s' has no method '%s'", c->name, mname);

    struct Paramstruct *p = m->paramlist;
    struct tnode *a = args;
    while(p != NULL && a != NULL){
        if(!typeCompatible(p->type, a->left->type) || exprIsPointer(a->left) != p->isPointer || a->left->ttype != p->ttype)
            typeErr("argument type mismatch in call to method '%s'", mname, "");
        p = p->next; a = a->right;
    }
    if(p != NULL || a != NULL) typeErr("argument count mismatch in call to method '%s'", mname, "");

    struct tnode *n = newNode(NODE_METHOD, m->type);
    n->left = obj;            /* the receiver */
    n->right = args;          /* same layout as NODE_CALL, so pushArgs() can be reused */
    n->Gentry = m->func;
    n->varname = strdup(mname);
    n->val = m->funcposition; /* stage 8 will need this for the vtable */
    return n;
}

struct tnode *makeNewNode(struct tnode *var, char *cname){
    if(!isClassType(var->type)) typeErr("new() needs a variable of a class type", "", "");
    struct Classtable *c = CLookup(cname);
    if(c == NULL) typeErr("unknown class '%s' in new()", cname, "");
    if(!classAssignable(var->type, c->type))
        typeErr("cannot assign an object of class '%s' to a variable of class '%s'", c->name, var->type->name);
    struct tnode *n = newNode(NODE_NEW, c->type);
    n->left = var;
    return n;
}

struct tnode *makeDeleteNode(struct tnode *var){
    if(!isClassType(var->type)) typeErr("delete() needs a variable of a class type", "", "");
    struct tnode *n = newNode(NODE_DELETE, TYPE_NONE);
    n->left = var;
    return n;
}