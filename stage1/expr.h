#ifndef EXPR_H
#define EXPR_H

typedef struct tnode{
    int val;
    char *op;
    struct tnode *left;
    struct tnode *right;
} tnode;

struct tnode *makeleafnode(int);
struct tnode *makeoperatornode(char, struct tnode *, struct tnode *);

#endif