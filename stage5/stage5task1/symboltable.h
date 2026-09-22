#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H
#define TYPE_INT 1
#define TYPE_STR 2
struct Paramstruct{
    int type;
    char *name;
    struct Paramstruct *next;
};
struct Gsymbol{
    char *name;
    int type;
    int size;
    int binding;
    int isArray;
    int cols;
    int isPointer;
    int isFunction;
    struct Paramstruct *paramlist;
    int flabel;
    struct Gsymbol *next;
};

struct Gsymbol *Lookup(char *name);
void InstallVar(char *name,int type,int size,int isArray,int cols,int isPointer);
void InstallFunc(char *name,int type,struct Paramstruct *paramlist);
struct Paramstruct *makeParam(int type, char *name);
struct Paramstruct *appendParam(struct Paramstruct *list, struct Paramstruct *p);
void PrintSymbolTable();

#endif
