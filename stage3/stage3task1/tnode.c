#include <stdlib.h>
#include <string.h>
#include "tnode.h"
#include <stdio.h>

#define TYPE_INT 1
struct tnode* createtree(int val,int type, int nodetype, char* varname,struct tnode* l, struct tnode* r)
{
    struct tnode* temp = malloc(sizeof(struct tnode));

    if(nodetype=='+' || nodetype=='*'){
        if(l->type!=TYPE_INT || r->type !=TYPE_INT){
            printf("Type error: invalid operands\n");
            exit(1);
        }
        type=TYPE_INT;
    }
    if(nodetype=='='){
        if(l->type!=r->type){
            printf("Type mimatch in assignment\n");
            exit(1);
        }
        type=TYPE_INT;
    }
    if(nodetype=='R' || nodetype=='W' || nodetype=='C'){
        type=TYPE_INT;
    }
    temp->val = val;
    temp->type=type;
    temp->nodetype = nodetype;

    if(varname != NULL)
        temp->varname = strdup(varname);  
    else
        temp->varname = NULL;

    temp->left = l;
    temp->right = r;

    return temp;
}