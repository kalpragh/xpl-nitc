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

struct Classtable *CInstall(char *name, char *parent){
    if(parent != NULL){
        printf("Error: class extension is not supported yet (class %s)\n", name);
        exit(1);
    }
    struct Classtable *c = calloc(1, sizeof *c);
    c->name = strdup(name);
    c->type = TInstallClass(name);        /* errors on duplicate names */
    c->type->cls = c;
    c->classIndex = classCount++;
    if(Ctail) Ctail->next = c; else Chead = c;
    Ctail = c;
    return c;
}

struct Memberfunclist *Class_Mlookup(struct Classtable *c, char *name){
    for(struct Memberfunclist *m = c->vfunc; m; m = m->next)
        if(strcmp(m->name, name) == 0) return m;
    return NULL;
}
Fieldlist *Class_Flookup(struct Classtable *c, char *name){ return FLookup(c->type, name); }

void Class_Finstall(struct Classtable *c, struct Typetable *ftype, char *name){
    if(c->methodCount > 0){
        printf("Error: field %s must be declared before the methods of class %s\n", name, c->name);
        exit(1);
    }
    FInstall(c->type, name, ftype);       /* duplicate / "too many member fields" checks */
}

void Class_Minstall(struct Classtable *c, char *name, struct Typetable *rtype, struct Paramstruct *params){
    if(Class_Mlookup(c, name) || Class_Flookup(c, name)){
        printf("Error: %s is already declared in class %s\n", name, c->name); exit(1);
    }
    if(c->methodCount >= MAX_METHODS){
        printf("Error: too many methods in class %s\n", c->name); exit(1);
    }
    if(isClassType(rtype)){
        printf("Error: method %s cannot return an object\n", name); exit(1);
    }
    char mangled[256];
    snprintf(mangled, sizeof mangled, "%s.%s", c->name, name);
    InstallFunc(mangled, rtype, params);
    struct Gsymbol *g = Lookup(mangled);
    g->owner = c;

    struct Memberfunclist *m = calloc(1, sizeof *m);
    m->name = strdup(name);
    m->type = rtype;
    m->paramlist = params;
    m->funcposition = c->methodCount++;
    m->flabel = g->flabel;
    m->func = g;
    struct Memberfunclist **p = &c->vfunc;
    while(*p) p = &(*p)->next;
    *p = m;
}

int classAssignable(struct Typetable *lhs, struct Typetable *rhs){
    return lhs == rhs;                    /* stage 8: walk rhs's parent chain */
}

void ClassTablePrint(void){
    for(struct Classtable *c = Chead; c; c = c->next){
        printf("class %s (index %d)\n", c->name, c->classIndex);
        for(Fieldlist *f = c->type->fields; f; f = f->next)
            printf("  field [%d] %s : %s\n", f->fieldIndex, f->name, f->type->name);
        for(struct Memberfunclist *m = c->vfunc; m; m = m->next)
            printf("  method [%d] %s -> %s  F%d\n", m->funcposition, m->name, m->type->name, m->flabel);
    }
}