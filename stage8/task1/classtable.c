#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "classtable.h"

extern int nextFlabel;
static struct Classtable *Chead = NULL, *Ctail = NULL;
static int classCount = 0;

struct Classtable *CLookup(char *name){
    for(struct Classtable *c = Chead; c; c = c->next)
        if(strcmp(c->name, name) == 0) return c;
    return NULL;
}


struct Memberfunclist *Class_Mlookup(struct Classtable *c, char *name){
    for(struct Memberfunclist *m = c->vfunc; m; m = m->next)
        if(strcmp(m->name, name) == 0) return m;
    return NULL;
}
Fieldlist *Class_Flookup(struct Classtable *c, char *name){ return FLookup(c->type, name); }
static void inheritFrom(struct Classtable *c, struct Classtable *p){
    for(Fieldlist *f = p->type->fields; f; f = f->next)
        FInstall(c->type, f->name, f->type);
    struct Memberfunclist **tail = &c->vfunc;
    for(struct Memberfunclist *m = p->vfunc; m; m = m->next){
        struct Memberfunclist *copy = calloc(1, sizeof *copy);
        *copy = *m;
        copy->next = NULL;
        *tail = copy;
        tail = &copy->next;
    }
    c->methodCount = p->methodCount;
}

static int sameSignature(struct Typetable *r1, struct Paramstruct *p1, struct Typetable *r2, struct Paramstruct *p2){
    if(r1 != r2) return 0;
    while(p1 && p2){
        if(p1->type != p2->type || p1->isPointer != p2->isPointer || p1->ttype != p2->ttype) return 0;
        p1 = p1->next;
        p2 = p2->next;
    }
    return p1 == NULL && p2 == NULL;
}

struct Classtable *CInstall(char *name, char *parent){
    struct Classtable *p = NULL;
    if(parent != NULL){
        p = CLookup(parent);
        if(p == NULL){
            printf("Error: parent class %s of class %s is not defined\n", parent, name);
            exit(1);
        }
    }
    struct Classtable *c = calloc(1, sizeof *c);
    c->name = strdup(name);
    c->type = TInstallClass(name);
    c->type->cls = c;
    c->classIndex = classCount++;
    c->parent = p;
    if(Ctail) Ctail->next = c; else Chead = c;
    Ctail = c;
    if(p != NULL) inheritFrom(c, p);
    return c;
}

void Class_Finstall(struct Classtable *c, struct Typetable *ftype, char *name){
    if(c->hasOwnMethod){
        printf("Error: field %s must be declared before the methods of class %s\n", name, c->name);
        exit(1);
    }
    if(Class_Mlookup(c, name)){
        printf("Error: %s is already declared in class %s\n", name, c->name);
        exit(1);
    }
    FInstall(c->type, name, ftype);
}

void Class_Minstall(struct Classtable *c, char *name, struct Typetable *rtype, struct Paramstruct *params){
    if(Class_Flookup(c, name)){
        printf("Error: %s is already declared in class %s\n", name, c->name);
        exit(1);
    }
    if(isClassType(rtype)){
        printf("Error: method %s cannot return an object\n", name);
        exit(1);
    }
    struct Memberfunclist *old = Class_Mlookup(c, name);
    if(old != NULL && old->func->owner == c){
        printf("Error: %s is already declared in class %s\n", name, c->name);
        exit(1);
    }
    if(old != NULL && !sameSignature(old->type, old->paramlist, rtype, params)){
        printf("Error: method %s in class %s must have the same signature as in its parent class\n", name, c->name);
        exit(1);
    }
    if(old == NULL && c->methodCount >= MAX_METHODS){
        printf("Error: too many methods in class %s\n", c->name);
        exit(1);
    }
    char mangled[256];
    snprintf(mangled, sizeof mangled, "%s.%s", c->name, name);
    InstallFunc(mangled, rtype, params);
    struct Gsymbol *g = Lookup(mangled);
    g->owner = c;
    c->hasOwnMethod = 1;

    if(old != NULL){
        old->paramlist = params;
        old->func = g;
        old->flabel = g->flabel;
        return;
    }

    struct Memberfunclist *m = calloc(1, sizeof *m);
    m->name = strdup(name);
    m->type = rtype;
    m->paramlist = params;
    m->funcposition = c->methodCount++;
    m->flabel = g->flabel;
    m->func = g;
    struct Memberfunclist **pp = &c->vfunc;
    while(*pp) pp = &(*pp)->next;
    *pp = m;
}

int classAssignable(struct Typetable *lhs, struct Typetable *rhs){
    if(!isClassType(lhs) || !isClassType(rhs)) return 0;
    for(struct Classtable *c = rhs->cls; c != NULL; c = c->parent)
        if(c->type == lhs) return 1;
    return 0;
}

void ClassTablePrint(void){
    for(struct Classtable *c = Chead; c; c = c->next){
        printf("class %s (index %d)\n", c->name, c->classIndex);
        if(c->parent) printf("  extends %s\n", c->parent->name);
        for(Fieldlist *f = c->type->fields; f; f = f->next)
            printf("  field [%d] %s : %s\n", f->fieldIndex, f->name, f->type->name);
        for(struct Memberfunclist *m = c->vfunc; m; m = m->next)
            printf("  method [%d] %s -> %s  F%d  (from %s)\n", m->funcposition, m->name, m->type->name, m->flabel, m->func->owner->name);
    }
}