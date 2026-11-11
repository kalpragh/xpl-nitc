#ifndef TNODE_H
#define TNODE_H
extern int libMode;
#include "typetable.h"
void writeLibraryDispatcher(void);
#define NODE_LE 5
#define NODE_GE 6
#define NODE_EQ 7
#define NODE_NE 8
#define NODE_ARRAY 9
#define NODE_ARRAY2D 10
#define NODE_ADDR 11
#define NODE_DEREF 12
#define NODE_CALL 13
#define NODE_ARG 14
#define NODE_RETURN 15
#define NODE_FIELD 16
#define NODE_STRCONST 17
#define NODE_NULL 18
#define NODE_ALLOC 19
#define NODE_FREE 20
#define NODE_INIT 21
#define NODE_NEW 22
#define NODE_DELETE 23
#define NODE_METHOD 24
struct tnode *makeSelfNode(void);
struct tnode *makeMethodCallNode(struct tnode *obj, char *mname, struct tnode *args);
struct tnode *makeNewNode(struct tnode *var, char *cname);
struct tnode *makeDeleteNode(struct tnode *var);
struct TupleType;
struct Gsymbol;
struct Lsymbol;

struct tnode {
    int val;
    struct Typetable *type;
    int nodetype;
    char *varname;
    struct Gsymbol *Gentry;
    struct Lsymbol *Lentry;
    struct TupleType *ttype;
    Fieldlist *field;
    struct tnode *left;
    struct tnode *middle;
    struct tnode *right;
};

struct tnode *createtree(int val, struct Typetable *type, int nodetype, char *varname,
                         struct tnode *l, struct tnode *m, struct tnode *r);
struct tnode *makeArgNode(struct tnode *expr);
struct tnode *appendArg(struct tnode *list, struct tnode *p);
struct tnode *makeFieldNode(struct tnode *base, char *fname);
struct tnode *makeTupleFieldNode(char *var, char *field);
struct tnode *makeNullNode(void);
struct tnode *makeInitNode(void);
struct tnode *makeAllocNode(struct tnode *var);
struct tnode *makeFreeNode(struct tnode *var);
int userEqOK(struct Typetable *a, struct Typetable *b);

void codegenStmt(struct tnode *t);
void writeArrayErrorHandler();
void writeHeader();

#endif