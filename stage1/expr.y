%{
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

int yylex();
int yyparse();
void yyerror(char* s);

#include "expr.h"
struct tnode* root;
FILE *target_file;
void prefix(struct tnode *);
void postfix(struct tnode *);
%}

%union{
    struct tnode *node;
}

%token <node> NUM
%type <node> expr start

%%
start : expr '\n' { root=$1; YYACCEPT; }
        ;

expr: '+' expr expr {$$= makeoperatornode('+', $2, $3); }
| '-' expr expr {$$= makeoperatornode('-', $2, $3); }
| '*' expr expr {$$= makeoperatornode('*', $2, $3); }
| '/' expr expr {$$= makeoperatornode('/', $2, $3); }
| NUM {$$= $1; }
;
%%
struct tnode *makeleafnode(int n){
    struct tnode* temp=(struct tnode*)malloc(sizeof(struct tnode));
    temp->val=n;
    temp->op=NULL;
    temp->left=NULL;
    temp->right=NULL;
    return temp;
}

struct tnode* makeoperatornode(char op, struct tnode* l, struct tnode* r){
    struct tnode* temp=(struct tnode*)malloc(sizeof(struct tnode));
    temp->val=0;
    temp->op=(char*) malloc(2);
    temp->op[0]=op;
    temp->op[1]='\0';
    temp->left=l;
    temp->right=r;
    return temp;
}

void prefix(struct tnode* t){
    if(t==NULL)return;
    if(t->op==NULL)printf("%d ", t->val);
    else printf("%s ", t->op);
    prefix(t->left);
    prefix(t->right);
}
void postfix(struct tnode* t){
    if(t==NULL)return;
    postfix(t->left);
    postfix(t->right);
    if(t->op==NULL)printf("%d ", t->val);
    else printf("%s ", t->op);
}
void yyerror(char* s){
    printf("%s\n", s);
}
int nextfree=0;
int getreg(){
    int reg=nextfree;
    nextfree++;
    return reg;
}
void freereg(){
    nextfree--;
}
int codegen(struct tnode *t){
    if(t->op==NULL){
        int r=getreg();
        fprintf(target_file, "MOV R%d, %d\n",r,t->val);
        return r;
    }
    else{
        int p=codegen(t->left);
        int q=codegen(t->right);
        if(t->op[0]=='+'){
            fprintf(target_file, "ADD R%d, R%d\n",p,q);
        }
        else if(t->op[0]=='-'){
            fprintf(target_file, "SUB R%d, R%d\n",p,q);
        }
        else if(t->op[0]=='*'){
            fprintf(target_file, "MUL R%d, R%d\n",p,q);
        }
        else if(t->op[0]=='/'){
            fprintf(target_file, "DIV R%d, R%d\n",p,q);
        }
        freereg();
        return p;
    }
}
void writeResult(int reg){
    fprintf(target_file, "MOV [4096], R%d\n", reg);

    /* Write the result via library call */
    fprintf(target_file, "MOV R2, \"Write\"\n");
    fprintf(target_file, "PUSH R2\n");
    fprintf(target_file, "MOV R2, -2\n");
    fprintf(target_file, "PUSH R2\n");
    fprintf(target_file, "PUSH R%d\n", reg);
    fprintf(target_file, "PUSH R0\n");
    fprintf(target_file, "PUSH R0\n");
    fprintf(target_file, "CALL 0\n");
    fprintf(target_file, "POP R0\n");
    fprintf(target_file, "POP R0\n");
    fprintf(target_file, "POP R0\n");
    fprintf(target_file, "POP R0\n");
    fprintf(target_file, "POP R0\n");

    /* Exit cleanly via library call */
    fprintf(target_file, "MOV R2, \"Exit\"\n");
    fprintf(target_file, "PUSH R2\n");
    fprintf(target_file, "PUSH R0\n");
    fprintf(target_file, "PUSH R0\n");
    fprintf(target_file, "PUSH R0\n");
    fprintf(target_file, "PUSH R0\n");
    fprintf(target_file, "CALL 0\n");
}

int main(){
    yyparse();
    printf("Prefix: ");
    prefix(root);
    printf("\n");
    printf("Postfix: ");
    postfix(root);
    printf("\n");
    target_file = fopen("out.xsm", "w");
    fprintf(target_file," %d\n %d\n %d\n %d\n %d\n %d\n %d\n %d\n ",0,2056,0,0,0,0,0,1);
    int resultReg = codegen(root);
    writeResult(resultReg);
    fclose(target_file);
    return 0;
}