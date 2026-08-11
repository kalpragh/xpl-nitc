#include <stdlib.h>
#include <string.h>
#include "tnode.h"
#include <stdio.h>

extern FILE *fp;
int reg=-1;
int label=0;
int getreg(){
    return ++reg;
}
void freereg(){
    reg--;
}
int getlabel(){
    return label++;
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
            int addr = 4096 + (t->varname[0] - 'a');
            fprintf(fp, "MOV R%d, [%d]\n", r, addr);
            return r;
        }

        case '=': {
            int r = codegen(t->right);

            int addr = 4096 + (t->left->varname[0] - 'a');

            fprintf(fp, "MOV [%d], R%d\n", addr, r);

            freereg();

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
            fprintf(fp,"MUL R%d R%d\n",r1,r2);
            freereg();
            return r1;
        }
        case '/':{
            int r1=codegen(t->left);
            int r2=codegen(t->right);
            fprintf(fp,"DIV R%d R%d\n",r1,r2);
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
        case 'L':{
            int label1=getlabel();
            int label2=getlabel();
            fprintf(fp, "L%d:\n",label1);
            int r=codegen(t->left);
            fprintf(fp, "JZ R%d, L%d\n",r,label2);
            freereg();
            codegen(t->middle);
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
       case 'R':
        {
            int addr = 4096 + (t->left->varname[0] - 'a');

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

        }
      }

     return -1;
    }
struct tnode* createtree(int val,int type, int nodetype, char* varname,struct tnode* l, struct tnode *m, struct tnode* r)
{
    struct tnode* temp = malloc(sizeof(struct tnode));

    if(nodetype=='N'){
        type=TYPE_INT;
    }
    else if(nodetype=='V'){
        type=TYPE_INT;
    }
    else if(nodetype=='+' || nodetype=='*'|| nodetype == '-' || nodetype == '/'){ //arithmatic
        if(l->type!=TYPE_INT || r->type !=TYPE_INT){
            printf("Type error: invalid operands\n");
            exit(1);
        }
        type=TYPE_INT;
    }
    else if(nodetype=='<'){
        if(l->type!=TYPE_INT || r->type!=TYPE_INT){
            printf("Type error in comparison\n");
            exit(1);
        }
        type=TYPE_BOOL; 
    }
    else if(nodetype=='='){
        if(l->type!=r->type){
            printf("Type misatch in assignment\n");
            exit(1);
        }
        type=TYPE_NONE;
    }
     else if(nodetype == 'I'){
        if(l->type != TYPE_BOOL){
            printf("Type error: IF condition must be boolean\n");
            exit(1);
        }
        type = TYPE_NONE;
    }

    else if(nodetype == 'L'){
        if(l->type != TYPE_BOOL){
            printf("Type error: WHILE condition must be boolean\n");
            exit(1);
        }
        type = TYPE_NONE;
    }

    else if(nodetype=='R' || nodetype=='W' || nodetype=='C'){
        type=TYPE_NONE;
    }
    temp->val = val;
    temp->type=type;
    temp->nodetype = nodetype;

    if(varname != NULL)
        temp->varname = strdup(varname);  
    else
        temp->varname = NULL;

    temp->left = l;
    temp->middle=m;
    temp->right = r;

    return temp;
}