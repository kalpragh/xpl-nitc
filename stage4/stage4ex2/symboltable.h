#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H
#define TYPE_INT 1
#define TYPE_STR 2
struct Gsymbol{
    char *name;
    int type;
    int size;
    int binding;
    int isArray;
    int cols;
    int isPointer;
    struct Gsymbol *next;
};

struct Gsymbol *Lookup(char *name);
void Install(char *name,int type,int size,int isArray,int cols,int isPointer);
void PrintSymbolTable();

#endif
