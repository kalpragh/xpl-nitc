#include <stdlib.h>
#include <string.h>
#include "tnode.h"
#include <stdio.h>

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
    else if(nodetype=='<' || nodetype=='>' || nodetype==NODE_LE || nodetype==NODE_GE || nodetype==NODE_EQ || nodetype==NODE_NE){
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