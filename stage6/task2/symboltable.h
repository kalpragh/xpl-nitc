#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include "typetable.h"

struct tnode;
struct Gsymbol;

struct Field {
    char *name;
    struct Typetable *type;
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

struct Paramstruct {
    struct Typetable *type;
    char *name;
    int isPointer;
    struct TupleType *ttype;
    struct Paramstruct *next;
};

struct Lsymbol {
    char *name;
    struct Typetable *type;
    int binding;
    int isPointer;
    struct TupleType *ttype;
    struct Lsymbol *next;
};

struct Gsymbol {
    char *name;
    struct Typetable *type;
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

struct Paramstruct *makeTupleParam(char *tname, char *name, int isPointer);
void InstallTupleFunc(char *name, struct TupleType *tt, int isPointer, struct Paramstruct *paramlist);
int FuncRetWords(struct Gsymbol *f);
void LInstallTuple(char *name, struct TupleType *tt, int isPointer, int isParam);
struct Field *makeField(struct Typetable *type, char *name);
struct Field *appendField(struct Field *list, struct Field *f);
struct TupleType *InstallTupleType(char *name, struct Field *fields);
struct TupleType *LookupTupleType(char *name);
struct Field *FindField(struct TupleType *tt, char *fname);
void InstallTupleVar(char *name, struct TupleType *tt, int isPointer);

void FreeLocalTable(struct Lsymbol *list);
int GetLocalCount(void);
struct Gsymbol *Lookup(char *name);
void InstallVar(char *name, struct Typetable *type, int size, int isArray, int cols, int isPointer);
void InstallFunc(char *name, struct Typetable *type, struct Paramstruct *paramlist);
struct Paramstruct *makeParam(struct Typetable *type, char *name, int isPointer);
struct Paramstruct *appendParam(struct Paramstruct *list, struct Paramstruct *p);
void PrintSymbolTable();
struct Lsymbol *LLookup(char *name);
void LInstallVar(char *name, struct Typetable *type, int isPointer, int isParam);
void SetCurrentFunction(struct Gsymbol *func);
void ClearLocalTable(void);
int LookupVar(char *name, struct Lsymbol **lentry, struct Gsymbol **gentry);
int CheckFunctionDefinition(struct Gsymbol *func, struct Paramstruct *defparams);
void InstallParams(struct Paramstruct *params);
struct Gsymbol *GetCurrentFunction(void);

#endif