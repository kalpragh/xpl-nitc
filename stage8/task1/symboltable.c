#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "symboltable.h"
#include"classtable.h"
struct Gsymbol *Ghead = NULL;
struct Lsymbol *Lhead = NULL;
struct Gsymbol *CurrentFunction = NULL;
int nextBinding = 4096;
int nextFlabel = 0;
static int nextParamBinding;
static int nextLocalBinding;
int GetLocalCount(void){
    return nextLocalBinding-1;
}
struct Gsymbol *GetCurrentFunction(void){
    return CurrentFunction;
}
struct Gsymbol *Lookup(char *name){
    struct Gsymbol *temp = Ghead;
    while(temp != NULL){
        if(strcmp(temp->name, name) == 0) return temp;
        temp = temp->next;
    }
    return NULL;
}
struct Lsymbol *LLookup(char *name){
    struct Lsymbol *temp=Lhead;
    while(temp!=NULL){
        if(strcmp(temp->name,name)==0){
            return temp;
        }
        temp=temp->next;
    }
    return NULL;
}
static void appendToGST(struct Gsymbol *temp){
    temp->next = NULL;
    if(Ghead == NULL){
        Ghead = temp;
    } else {
        struct Gsymbol *p = Ghead;
        while(p->next != NULL) p = p->next;
        p->next = temp;
    }
}


void LInstallVar(char *name, struct Typetable *type, int isPointer, int isParam){
    if(!isParam && type != NULL && type->isClass && CurrentFunction != NULL && CurrentFunction->owner != NULL){
        printf("Error: %s: methods cannot declare local variables of class type\n", name);
        exit(1);
    }
    if(LLookup(name) != NULL){
        printf("Error: %s already declared in local scope\n", name);
        exit(1);
    }
    struct Lsymbol *temp = malloc(sizeof(struct Lsymbol));
    temp->name = strdup(name);
    temp->type = type;
    temp->isPointer = isPointer;
    temp->binding = isParam ? nextParamBinding-- : nextLocalBinding++;
    temp->next = NULL;
    temp->ttype=NULL;
    if(Lhead == NULL) Lhead = temp;
    else { struct Lsymbol *p = Lhead; while(p->next) p = p->next; p->next = temp; }
}
void ClearLocalTable(void)
{
    Lhead = NULL;
    nextParamBinding=-3;
    nextLocalBinding=1;
}
void FreeLocalTable(struct Lsymbol *list){
    while(list != NULL){
        struct Lsymbol *next = list->next;
        free(list->name);
        free(list);
        list = next;
    }
}
void SetCurrentFunction(struct Gsymbol *func)
{
    CurrentFunction = func;
}
void InstallVar(char *name, struct Typetable *type, int size, int isArray, int cols, int isPointer){
    if(Lookup(name) != NULL){
        printf("Error: %s already declared\n", name);
        exit(1);
    }
    struct Gsymbol *temp = calloc(1, sizeof(struct Gsymbol));
    temp->name = strdup(name);
    temp->type = type;
    temp->size = size;
    temp->binding = nextBinding;
    temp->isArray = isArray;
    temp->cols = cols;
    temp->isPointer = isPointer;
    temp->isFunction = 0;
    temp->isDefined=0;
    temp->paramlist = NULL;
    temp->flabel = -1;
    temp->ttype=NULL;
    nextBinding += size;
    temp->funcbody=NULL;
    appendToGST(temp);
}

void InstallFunc(char *name, struct Typetable *type, struct Paramstruct *paramlist){
    if(Lookup(name) != NULL){
        printf("Error: %s already declared\n", name);
        exit(1);
    }
    struct Gsymbol *temp = calloc(1, sizeof(struct Gsymbol));
    temp->name = strdup(name);
    temp->type = type;
    temp->size = 0;
    temp->binding = -1;
    temp->isArray = 0;
    temp->cols = 0;
    temp->isPointer = 0;
    temp->isFunction = 1;
    temp->isDefined=0;
    temp->paramlist = paramlist;
    temp->flabel = nextFlabel++;
    temp->ttype=NULL;
    temp->funcbody=NULL;
    appendToGST(temp);
}

struct Paramstruct *makeParam(struct Typetable *type, char *name, int isPointer){
    struct Paramstruct *p = malloc(sizeof(struct Paramstruct));
    p->type = type;
    p->name = strdup(name);
    p->isPointer=isPointer;
    p->ttype=NULL;
    p->next = NULL;
    return p;
}

struct Paramstruct *appendParam(struct Paramstruct *list, struct Paramstruct *p){
    if(list == NULL) return p;
    struct Paramstruct *q = list;
    while(q->next != NULL) q = q->next;
    q->next = p;
    return list;
}

static const char* typeName(struct Typetable *t){
    return t ? t->name : "?";
}
int LookupVar(char *name,struct Lsymbol **lentry,struct Gsymbol **gentry){
    *lentry=LLookup(name);
    if(*lentry!=NULL){
        *gentry=NULL;
        return 1;
    }
    *gentry=Lookup(name);
    if(*gentry!=NULL){
        return 2;
    }
    return 0;
}
void PrintSymbolTable(){
    struct Gsymbol *temp = Ghead;
    printf("Global Symbol Table\n");
    while(temp != NULL){
        if(temp->isFunction){
            printf("FUNC  %s\treturn=%s\tflabel=F%d\tparams=(",
                   temp->name, typeName(temp->type), temp->flabel);
            struct Paramstruct *p = temp->paramlist;
            while(p != NULL){
                printf("%s %s", typeName(p->type), p->name);
                if(p->next != NULL) printf(", ");
                p = p->next;
            }
            printf(")\n");
        } else {
            printf("VAR   %s\ttype=%s\tsize=%d\tbinding=%d\tisArray=%d\tisPointer=%d\n",
                   temp->name, typeName(temp->type), temp->size,
                   temp->binding, temp->isArray, temp->isPointer);
        }
        temp = temp->next;
    }
}
int CheckFunctionDefinition(struct Gsymbol *func, struct Paramstruct *defparams){
    struct Paramstruct *decl=func->paramlist;
    struct Paramstruct *def=defparams;

    while(decl!=NULL && def!=NULL){
        if(strcmp(decl->name,def->name)!=0){
            printf("Error: parameter name mismatch in function %s\n",func->name);
            printf("Expected: %s, Found: %s\n",decl->name,def->name);
            return 0;
        }
        if(decl->type!=def->type || decl->isPointer!=def->isPointer || decl->ttype!=def->ttype){
            printf("Error: parameter type mismatch in function %s\n",func->name);
            printf("Parameter: %s\n",decl->name);
            return 0;
        }
        decl=decl->next;
        def=def->next;
    }
    if(decl!=NULL || def!=NULL){
        printf("Error: number of parameters mismatch in function %s\n",func->name);
        return 0;
    }
    return 1;
}
void InstallParams(struct Paramstruct *params){
    nextParamBinding = -(2 + FuncRetWords(CurrentFunction));
    for(struct Paramstruct *p = params; p != NULL; p = p->next){
        if(p->ttype != NULL) LInstallTuple(p->name, p->ttype, p->isPointer, 1);
        else LInstallVar(p->name, p->type, p->isPointer, 1);
    }
    if(CurrentFunction->owner != NULL) LInstallSelf(CurrentFunction->owner->type);
}
static struct TupleType *Thead = NULL;

struct Field *makeField(struct Typetable *type, char *name){
    struct Field *f = malloc(sizeof(struct Field));
    f->type = type;
    f->name = strdup(name);
    f->pos = 0;
    f->next = NULL;
    return f;
}
struct Field *appendField(struct Field *list, struct Field *f){
    if(list == NULL) return f;
    struct Field *q = list;
    while(q->next) q = q->next;
    q->next = f;
    return list;
}
struct TupleType *LookupTupleType(char *name){
    for(struct TupleType *t = Thead; t; t = t->next)
        if(strcmp(t->name, name) == 0) return t;
    return NULL;
}
struct TupleType *InstallTupleType(char *name, struct Field *fields){
    if(LookupTupleType(name) != NULL){
        printf("Error: tuple type %s already defined\n", name);
        exit(1);
    }
    int n = 0;
    for(struct Field *f = fields; f; f = f->next){
        for(struct Field *g = fields; g != f; g = g->next)
            if(strcmp(f->name, g->name) == 0){
                printf("Error: duplicate field %s in tuple %s\n", f->name, name);
                exit(1);
            }
        f->pos = n++;
    }
    struct TupleType *t = malloc(sizeof(struct TupleType));
    t->name = strdup(name);
    t->fields = fields;
    t->size = n;
    t->next = Thead;
    Thead = t;
    return t;
}
struct Field *FindField(struct TupleType *tt, char *fname){
    for(struct Field *f = tt->fields; f; f = f->next)
        if(strcmp(f->name, fname) == 0) return f;
    return NULL;
}
void InstallTupleVar(char *name, struct TupleType *tt, int isPointer){
    if(Lookup(name) != NULL){
        printf("Error: %s already declared\n", name);
        exit(1);
    }
    struct Gsymbol *temp = calloc(1,sizeof(struct Gsymbol));
    temp->name = strdup(name);
    temp->type = TYPE_TUPLE;
    temp->ttype = tt;
    temp->size = isPointer ? 1 : tt->size;
    temp->binding = nextBinding;
    temp->isArray = 0;
    temp->cols = 0;
    temp->isPointer = isPointer;
    temp->isFunction = 0;
    temp->isDefined = 0;
    temp->paramlist = NULL;
    temp->flabel = -1;
    temp->funcbody = NULL;
    temp->numLocals = 0;
    temp->locals = NULL;
    nextBinding += temp->size;
    appendToGST(temp);
}
void LInstallTuple(char *name, struct TupleType *tt, int isPointer, int isParam){
    if(LLookup(name) != NULL){
        printf("Error: %s already declared in local scope\n", name);
        exit(1);
    }
    int size = isPointer ? 1 : tt->size;
    struct Lsymbol *temp = malloc(sizeof(struct Lsymbol));
    temp->name = strdup(name);
    temp->type = TYPE_TUPLE;
    temp->ttype = tt;
    temp->isPointer = isPointer;
    if(isParam){
        temp->binding = nextParamBinding - (size - 1);
        nextParamBinding -= size;
    } else {
        temp->binding = nextLocalBinding;
        nextLocalBinding += size;
    }
    temp->next = NULL;
    if(Lhead == NULL) Lhead = temp;
    else { struct Lsymbol *p = Lhead; while(p->next) p = p->next; p->next = temp; }
    
}
struct Paramstruct *makeTupleParam(char *tname, char *name, int isPointer){
    struct TupleType *tt = LookupTupleType(tname);
    if(tt == NULL){ printf("Error: unknown tuple type %s\n", tname); exit(1); }
    struct Paramstruct *p = malloc(sizeof(struct Paramstruct));
    p->type = TYPE_TUPLE;
    p->name = strdup(name);
    p->isPointer = isPointer;
    p->ttype = tt;
    p->next = NULL;
    return p;
}
void InstallTupleFunc(char *name, struct TupleType *tt, int isPointer, struct Paramstruct *paramlist){
    InstallFunc(name, TYPE_TUPLE, paramlist);
    struct Gsymbol *f = Lookup(name);
    f->ttype = tt;
    f->isPointer = isPointer;
}
int FuncRetWords(struct Gsymbol *f){
    if(f != NULL && f->type == TYPE_TUPLE && !f->isPointer && f->ttype != NULL)
        return f->ttype->size;
    return 1;
}
void LInstallSelf(struct Typetable *classType){
    struct Lsymbol *temp = calloc(1, sizeof(struct Lsymbol));
    temp->name = strdup("self");
    temp->type = classType;
    temp->binding = nextParamBinding--;
    if(Lhead == NULL) Lhead = temp;
    else { struct Lsymbol *p = Lhead; while(p->next) p = p->next; p->next = temp; }
}