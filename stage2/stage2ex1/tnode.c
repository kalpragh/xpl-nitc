#include <stdlib.h>
#include <string.h>
#include "tnode.h"

struct tnode* createtree(int val, int nodetype, char* varname,
                         struct tnode* l, struct tnode* r)
{
    struct tnode* temp = malloc(sizeof(struct tnode));

    temp->val = val;
    temp->nodetype = nodetype;

    if(varname != NULL)
        temp->varname = strdup(varname);   // ✅ IMPORTANT
    else
        temp->varname = NULL;

    temp->left = l;
    temp->right = r;

    return temp;
}