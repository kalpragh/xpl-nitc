#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "symboltable.h"

struct Gsymbol *Ghead = NULL;
int nextBinding = 4096;
int nextFlabel = 0;

struct Gsymbol *Lookup(char *name){
    struct Gsymbol *temp = Ghead;
    while(temp != NULL){
        if(strcmp(temp->name, name) == 0) return temp;
        temp = temp->next;
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
    temp->paramlist = NULL;
    temp->flabel = -1;
    nextBinding += size;
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
    temp->paramlist = paramlist;
    temp->flabel = nextFlabel++;
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