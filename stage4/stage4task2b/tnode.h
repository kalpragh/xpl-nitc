#ifndef TNODE_H
#define TNODE_H
#define TYPE_NONE -1
#define TYPE_BOOL 0
#define TYPE_INT  1
#define TYPE_STR 2

#define NODE_LE 5
#define NODE_GE 6
#define NODE_EQ 7
#define NODE_NE 8
#define NODE_ARRAY 9
void writeArrayErrorHandler();
void writeHeader();
struct Gsymbol;
struct tnode {
    int val;
    int type;
    int nodetype;
    char *varname;

    struct Gsymbol *Gentry;

    struct tnode *left;
    struct tnode *middle;
    struct tnode *right;
};

struct tnode* createtree(int val,int type, int nodetype, char* varname,
                         struct tnode* l, struct tnode* m,struct tnode* r);

#endif