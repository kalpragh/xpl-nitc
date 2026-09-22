#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "symboltable.h"

struct Gsymbol *Ghead = NULL;
struct Lsymbol *Lhead = NULL;
struct Gsymbol *CurrentFunction = NULL;
int nextBinding = 4096;
int nextFlabel = 0;

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
static int nextParamBinding;
static int nextLocalBinding;

void LInstallVar(char *name, int type, int isPointer, int isParam){
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
void InstallVar(char *name, int type, int size, int isArray, int cols, int isPointer){
    if(Lookup(name) != NULL){
        printf("Error: %s already declared\n", name);
        exit(1);
    }
    struct Gsymbol *temp = malloc(sizeof(struct Gsymbol));
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
    nextBinding += size;
    temp->funcbody=NULL;
    appendToGST(temp);
}

void InstallFunc(char *name, int type, struct Paramstruct *paramlist){
    if(Lookup(name) != NULL){
        printf("Error: %s already declared\n", name);
        exit(1);
    }
    struct Gsymbol *temp = malloc(sizeof(struct Gsymbol));
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
    temp->funcbody=NULL;
    appendToGST(temp);
}

struct Paramstruct *makeParam(int type, char *name){
    struct Paramstruct *p = malloc(sizeof(struct Paramstruct));
    p->type = type;
    p->name = strdup(name);
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

static const char* typeName(int t){
    if(t == TYPE_INT) return "INT";
    if(t == TYPE_STR) return "STR";
    return "?";
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
        if(decl->type!=def->type){
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
    struct Paramstruct *p=params;
    while(p!=NULL){
        LInstallVar(p->name,p->type,0,1);
        p=p->next;
    }
}