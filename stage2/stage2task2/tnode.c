#include <stdlib.h>
#include <string.h>
#include "tnode.h"

struct tnode* createtree(int val, int nodetype, char* varname,
                         struct tnode* l, struct tnode* r) {

    struct tnode* temp = (struct tnode*)malloc(sizeof(struct tnode));

    temp->val = val;
    temp->nodetype = nodetype;

    if (varname)
        temp->varname = strdup(varname);
    else
        temp->varname = NULL;

    temp->left = l;
    temp->right = r;

    return temp;
}