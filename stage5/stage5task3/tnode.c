#include <stdlib.h>
#include <string.h>
#include "tnode.h"
#include <stdio.h>
#include "symboltable.h"
extern FILE *fp;
#define MAX_LOOP_DEPTH 100
int loopStartStack[MAX_LOOP_DEPTH];
int loopEndStack[MAX_LOOP_DEPTH];
int loopDepth = 0;
int label=0;
int nextfree=0;
int arrayErrorLabel=-1;
int codegen(struct tnode *t);
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
    fprintf(fp, "MOV SP, 4122\n");
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
    return n;
}
struct tnode *appendArg(struct tnode *list, struct tnode *p){
    if(list == NULL) return p;
    struct tnode *q = list;
    while(q->right != NULL) q = q->right;
    q->right = p;
    return list;
}
int codegenaddr2d(struct tnode *t){
    int r=getreg();
    fprintf(fp,"MOV R%d, %d\n",r,t->left->Gentry->binding);

    int rrow=codegen(t->middle);
    int rows = t->left->Gentry->size / t->left->Gentry->cols;

    if(t->middle->nodetype != 'N'){
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

    if(t->right->nodetype != 'N'){
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
struct tnode* createtree(int val,int type, int nodetype, char* varname,struct tnode* l, struct tnode *m, struct tnode* r)
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
    temp->Lentry=NULL;

    if(nodetype=='V'){
        struct Lsymbol *lentry=LLookup(varname);
        if(lentry!=NULL){
            temp->Lentry=lentry;
            temp->type=lentry->type;
        }
        else {
            struct Gsymbol *gentry=Lookup(varname);
            if(gentry==NULL){
                printf("Error: Variable %s not declared",varname);
                exit(1);
            }
            temp->Gentry=gentry;
            temp->type=gentry->type;
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
        if(l->type!=TYPE_INT || r->type!=TYPE_INT){
            printf("Type error in comparison\n");
            exit(1);
        }
        temp->type=TYPE_BOOL; 
    }
    else if(nodetype=='='){
        if(l->type!=r->type){
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
        temp->type=l->Gentry->type;
        temp->Gentry=l->Gentry;
    }
    else if(nodetype==NODE_DEREF){
        temp->type=l->Gentry->type;
        temp->Gentry=l->Gentry;
    }
    else if(nodetype == NODE_CALL){
        struct Gsymbol *fentry = Lookup(varname);
        if(fentry == NULL || !fentry->isFunction){
            printf("Error: %s is not a function\n", varname);
            exit(1);
        }
        struct Paramstruct *p = fentry->paramlist;
        struct tnode *a = r;
        while(p != NULL && a != NULL){
            if(a->left->type != p->type){
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
    }
    else if(nodetype == NODE_RETURN){
        struct Gsymbol *cf = GetCurrentFunction();
        if(cf == NULL){
            printf("Error: return outside a function\n");
            exit(1);
        }
        if(l->type != cf->type){
            printf("Error: return type mismatch in function %s\n", cf->name);
            exit(1);
        }
        temp->type = TYPE_NONE;
    }
    return temp;
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
            int r = getreg();
            fprintf(fp, "MOV R%d, [%d]\n", r, t->Gentry->binding);
            return r;
        }

        case '=': {
            int r = codegen(t->right);

            if(t->left->nodetype == NODE_ARRAY)
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
                int addr=getreg();
                fprintf(fp, "MOV R%d, [%d]\n", addr, t->left->left->Gentry->binding);
                fprintf(fp, "MOV [R%d], R%d\n", addr, r);
                freereg();
                freereg();
            }
            else
            {
                fprintf(fp, "MOV [%d], R%d\n",
                        t->left->Gentry->binding, r);

                freereg();
            }

            return -1;
        }

        case 'C': {
            codegen(t->left);
            codegen(t->right);
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

            codegen(t->middle);

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
                codegen(t->middle);
                fprintf(fp,"L%d:\n",label);
            }
            else{
                //if-then-else
                int elselabel=getlabel();
                int endlabel=getlabel();
                fprintf(fp,"JZ R%d, L%d\n",r,elselabel);
                freereg();
                //then
                codegen(t->middle);
                fprintf(fp,"JMP L%d\n",endlabel);
                //else
                fprintf(fp,"L%d:\n",elselabel);
                codegen(t->right);
                fprintf(fp,"L%d:\n",endlabel);
            }
            return -1;
        }
       case 'R':
        {
            int addr;
            
            if(t->left->nodetype=='V'){
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
            int addr = getreg();
            fprintf(fp, "MOV R%d, [%d]\n", addr, t->left->left->Gentry->binding);

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

            codegen(t->middle);

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

            codegen(t->middle);

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
        case NODE_ADDR: {
            int r=getreg();
            fprintf(fp, "MOV R%d, %d\n",r,t->left->Gentry->binding);
            return r;
        }
        case NODE_DEREF: {
            int r=getreg();
            fprintf(fp, "MOV R%d, [%d]\n",r,t->left->Gentry->binding);
            fprintf(fp, "MOV R%d, [R%d]\n",r,r);
            return r;
        }
      }

     return -1;
    }