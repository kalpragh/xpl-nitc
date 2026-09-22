#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H
struct tnode;
#define TYPE_INT 1
#define TYPE_STR 2
struct Paramstruct{
    int type;
    char *name;
    struct Paramstruct *next;
};
struct Lsymbol{
    char *name;
    int type;
    int binding;
    int isPointer;
    struct Lsymbol *next;
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
    int isDefined;
    struct Paramstruct *paramlist;
    int flabel;
    struct tnode *funcbody;
    struct Gsymbol *next;
};

struct Gsymbol *Lookup(char *name);
void InstallVar(char *name,int type,int size,int isArray,int cols,int isPointer);
void InstallFunc(char *name,int type,struct Paramstruct *paramlist);
struct Paramstruct *makeParam(int type, char *name);
struct Paramstruct *appendParam(struct Paramstruct *list, struct Paramstruct *p);
void PrintSymbolTable();
struct Lsymbol *LLookup(char *name);
void LInstallVar(char *name, int type, int isPointer);
void SetCurrentFunction(struct Gsymbol *func);
void ClearLocalTable(void);
struct Gsymbol *LookupGlobal(char *name);
int LookupVar(char *name, struct Lsymbol **lentry,struct Gsymbol **gentry);
int CheckFunctionDefinition(struct Gsymbol *func, struct Paramstruct *defparams);
void InstallParams(struct Paramstruct *params);
struct Gsymbol *GetCurrentFunction(void);
#endif
