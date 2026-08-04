#ifndef TNODE_H
#define TNODE_H

typedef struct tnode {
    int val;            // for numbers
    int type;           // INT, etc (can ignore for now)
    char* varname;      // for variables
    int nodetype;       // operator / read / write / connector
    struct tnode *left, *right;
} tnode;

tnode* createtree(int val, int nodetype, char* varname, tnode* l, tnode* r);

#endif