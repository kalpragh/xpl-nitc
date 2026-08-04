#ifndef TNODE_H
#define TNODE_H

struct tnode {
    int val;
    int nodetype;
    char *varname;
    struct tnode *left;
    struct tnode *right;
};

struct tnode* createtree(int val, int nodetype, char* varname,
                         struct tnode* l, struct tnode* r);

#endif