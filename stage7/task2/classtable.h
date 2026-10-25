#ifndef CLASSTABLE_H
#define CLASSTABLE_H
#include "typetable.h"
#include "symboltable.h"
#define MAX_METHODS 8

struct Memberfunclist {
    char *name;
    struct Typetable *type;
    struct Paramstruct *paramlist;
    int funcposition;
    int flabel;
    struct Gsymbol *func;            /* hidden "Class.method" entry */
    struct Memberfunclist *next;
};
struct Classtable {
    char *name;
    struct Typetable *type;          /* fields live in type->fields */
    struct Memberfunclist *vfunc;
    struct Classtable *parent;       /* NULL until stage 8 */
    int classIndex;
    int methodCount;
    struct Classtable *next;
};

struct Classtable *CInstall(char *name, char *parent);
struct Classtable *CLookup(char *name);
void Class_Finstall(struct Classtable *c, struct Typetable *ftype, char *name);
void Class_Minstall(struct Classtable *c, char *name, struct Typetable *rtype, struct Paramstruct *params);
struct Memberfunclist *Class_Mlookup(struct Classtable *c, char *name);
Fieldlist *Class_Flookup(struct Classtable *c, char *name);
int classAssignable(struct Typetable *lhs, struct Typetable *rhs);
void ClassTablePrint(void);
#endif