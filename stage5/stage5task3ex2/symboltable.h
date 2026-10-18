#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H
struct tnode;
#define TYPE_INT 1
#define TYPE_STR 2
#define TYPE_TUPLE 3
struct Field {
    char *name;
    int type;            
    int pos;          
    struct Field *next;
};
struct TupleType {
    char *name;
    struct Field *fields;
    int size;           
    struct TupleType *next;
};
extern struct Lsymbol *Lhead;
struct Paramstruct{
    int type;
    char *name;
    int isPointer;
    struct Paramstruct *next;
};
struct Field *makeField(int type, char *name);
struct Field *appendField(struct Field *list, struct Field *f);
struct TupleType *InstallTupleType(char *name, struct Field *fields);
struct TupleType *LookupTupleType(char *name);
struct Field *FindField(struct TupleType *tt, char *fname);
void InstallTupleVar(char *name, struct TupleType *tt, int isPointer);
void LInstallTuple(char *name, struct TupleType *tt, int isPointer);
struct Lsymbol{
    char *name;
    int type;
    int binding;
    int isPointer;
    struct TupleType *ttype;
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
    int flabel;
    struct TupleType *ttype;
    struct Paramstruct *paramlist;
    int numLocals;    
    struct Lsymbol *locals;
    struct tnode *funcbody;
    struct Gsymbol *next;
};
void FreeLocalTable(struct Lsymbol *list);
int GetLocalCount(void);
struct Gsymbol *Lookup(char *name);
void InstallVar(char *name,int type,int size,int isArray,int cols,int isPointer);
void InstallFunc(char *name,int type,struct Paramstruct *paramlist);
struct Paramstruct *makeParam(int type, char *name,int isPointer);
struct Paramstruct *appendParam(struct Paramstruct *list, struct Paramstruct *p);
void PrintSymbolTable();
struct Lsymbol *LLookup(char *name);
void LInstallVar(char *name, int type, int isPointer, int isParam);
void SetCurrentFunction(struct Gsymbol *func);
void ClearLocalTable(void);
struct Gsymbol *LookupGlobal(char *name);
int LookupVar(char *name, struct Lsymbol **lentry,struct Gsymbol **gentry);
int CheckFunctionDefinition(struct Gsymbol *func, struct Paramstruct *defparams);
void InstallParams(struct Paramstruct *params);
struct Gsymbol *GetCurrentFunction(void);
#endif
