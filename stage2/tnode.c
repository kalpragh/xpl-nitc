#include<stdlib.h>
#include<string.h>
#include"tnode.h"

tnode *createtree(int val,int nodetype,char *varname,tnode *l,tnode *r){
    tnode *temp=(tnode *)malloc(sizeof(tnode));
    temp->val=val;
    temp->nodetype=nodetype;
    if(varname!=NULL){
        temp->varname=strdup(varname);
    } else {
        temp->varname=NULL;
    }
    temp->left=l;
    temp->right=r;
    return temp;
}