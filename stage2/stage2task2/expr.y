%{
#include<stdio.h>
#include<stdlib.h>
#include"tnode.h"


struct tnode *root;

int yylex(void);
int yyerror(char *s);

int codegen(struct tnode *t);
int getreg();
void freereg();
int getAddress(char var);
FILE *target_file;
%}

%union{
    int num;
    char id;
    struct tnode *node;
}
%token <num> NUM
%token <id> ID
%token START END READ WRITE

%left '+'
%left '*'

%type <node> Program Slist Stmt Expr
%%
Program:
    START Slist END { root = $2; }
;

Slist:
    Slist Stmt { $$ = createtree(0,'C',NULL,$1,$2); }
  | Stmt       { $$ = $1; }
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
      Expr '+' Expr { $$ = createtree(0,'+',NULL,$1,$3); }
    | Expr '*' Expr { $$ = createtree(0,'*',NULL,$1,$3); }
    | NUM           { $$ = createtree($1,NUM,NULL,NULL,NULL); }
    | ID            { $$ = createtree(0,ID,(char[]){$1,'\0'},NULL,NULL); }
;
%%

int codegen(struct tnode *t){

    int r1, r2;

    if(t->nodetype == NUM){
        r1 = getreg();
        fprintf(target_file, "MOV R%d, %d\n", r1, t->val);
        return r1;
    }

    if(t->nodetype == ID){
        r1 = getreg();
        int addr = getAddress(t->varname[0]);
        fprintf(target_file, "MOV R%d, [%d]\n", r1, addr);
        return r1;
    }

    if(t->nodetype == '+'){
        r1 = codegen(t->left);
        r2 = codegen(t->right);
        fprintf(target_file, "ADD R%d, R%d\n", r1, r2);
        freereg();
        return r1;
    }

    if(t->nodetype == '*'){
        r1 = codegen(t->left);
        r2 = codegen(t->right);
        fprintf(target_file, "MUL R%d, R%d\n", r1, r2);
        freereg();
        return r1;
    }

    if(t->nodetype == '='){
        r1 = codegen(t->right);
        int addr = getAddress(t->left->varname[0]);
        fprintf(target_file, "MOV [%d], R%d\n", addr, r1);
        return r1;
    }

    if(t->nodetype == READ){
        int addr = getAddress(t->left->varname[0]);

        fprintf(target_file, "MOV R2, \"Read\"\n");
        fprintf(target_file, "PUSH R2\n");
        fprintf(target_file, "MOV R2, -1\n");
        fprintf(target_file, "PUSH R2\n");
        fprintf(target_file, "MOV R2, %d\n", addr);
        fprintf(target_file, "PUSH R2\n");
        fprintf(target_file, "PUSH R0\n");
        fprintf(target_file, "PUSH R0\n");
        fprintf(target_file, "CALL 0\n");

        fprintf(target_file, "POP R0\n");
        fprintf(target_file, "POP R0\n");
        fprintf(target_file, "POP R0\n");
        fprintf(target_file, "POP R0\n");
        fprintf(target_file, "POP R0\n");

        return -1;
    }

    if(t->nodetype == WRITE){
        r1 = codegen(t->left);

        fprintf(target_file, "MOV R2, \"Write\"\n");
        fprintf(target_file, "PUSH R2\n");
        fprintf(target_file, "MOV R2, -2\n");
        fprintf(target_file, "PUSH R2\n");
        fprintf(target_file, "PUSH R%d\n", r1);
        fprintf(target_file, "PUSH R0\n");
        fprintf(target_file, "PUSH R0\n");
        fprintf(target_file, "CALL 0\n");

        fprintf(target_file, "POP R0\n");
        fprintf(target_file, "POP R0\n");
        fprintf(target_file, "POP R0\n");
        fprintf(target_file, "POP R0\n");
        fprintf(target_file, "POP R0\n");

        freereg();
        return -1;
    }

    if(t->nodetype == 'C'){
        codegen(t->left);
        codegen(t->right);
        return -1;
    }

    return -1;
}

int yyerror(char* s){
    printf("%s\n", s);
    return 1;
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
int getAddress(char var){
    return 4096 + (var - 'a');
}

int main(){
    yyparse();

    target_file = fopen("out.xsm", "w");

    fprintf(target_file,"0\n2056\n0\n0\n0\n0\n0\n1\n");

    codegen(root);

    fclose(target_file);

    return 0;
}