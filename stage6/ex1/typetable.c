#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "typetable.h"

Typetable *TINT, *TSTR, *TBOOL, *TVOID, *TTUPLE;
static Typetable *head = NULL, *tail = NULL;

static void tt_error(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "Error: ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end(ap);
    exit(1);
}

static Typetable *newType(const char *name, int isUser) {
    Typetable *t = calloc(1, sizeof *t);
    t->name = strdup(name);
    t->isUser = isUser;
    if (tail) tail->next = t; else head = t;
    tail = t;
    return t;
}

void TypeTableCreate(void) {
    TINT  = newType("int", 0);
    TSTR  = newType("str", 0);
    TBOOL = newType("$bool", 0);
    TVOID = newType("$void", 0);
    TTUPLE = newType("$tuple", 0);
}

Typetable *TLookup(const char *name) {
    for (Typetable *t = head; t; t = t->next)
        if (strcmp(t->name, name) == 0) return t;
    return NULL;
}

Typetable *TResolve(const char *name) {
    Typetable *t = TLookup(name);
    if (!t || !t->isUser) tt_error("undefined type '%s'", name);
    return t;
}

Typetable *TInstall(const char *name) {
    if (TLookup(name)) tt_error("type '%s' is already defined", name);
    return newType(name, 1);
}

Fieldlist *FLookup(Typetable *t, const char *fname) {
    for (Fieldlist *f = t->fields; f; f = f->next)
        if (strcmp(f->name, fname) == 0) return f;
    return NULL;
}

Fieldlist *FInstall(Typetable *t, const char *fname, Typetable *ftype) {
    if (ftype == TBOOL || ftype == TVOID)
        tt_error("field '%s' of type '%s' has an invalid type", fname, t->name);
    if (FLookup(t, fname))
        tt_error("duplicate field '%s' in type '%s'", fname, t->name);
    if (t->size >= MAX_FIELDS)
        tt_error("too many member fields in type '%s'", t->name);

    Fieldlist *f = calloc(1, sizeof *f);
    f->name = strdup(fname);
    f->type = ftype;
    f->fieldIndex = t->size++;

    Fieldlist **p = &t->fields;          /* append, keeping declaration order */
    while (*p) p = &(*p)->next;
    *p = f;
    return f;
}

int isUserType(Typetable *t) { return t && t->isUser; }

int typeCompatible(Typetable *lhs, Typetable *rhs) {
    return lhs == rhs || (isUserType(lhs) && rhs == TVOID);
}

void TypeTablePrint(void) {
    for (Typetable *t = head; t; t = t->next) {
        if (!t->isUser) continue;
        printf("type %s (%d field%s)\n", t->name, t->size, t->size == 1 ? "" : "s");
        for (Fieldlist *f = t->fields; f; f = f->next)
            printf("  [%d] %s : %s\n", f->fieldIndex, f->name, f->type->name);
    }
}