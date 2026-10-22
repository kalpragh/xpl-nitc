#ifndef TYPETABLE_H
#define TYPETABLE_H

#define MAX_FIELDS 8            

struct Typetable;

typedef struct Fieldlist {
    char *name;
    int fieldIndex;             
    struct Typetable *type;
    struct Fieldlist *next;
} Fieldlist;

typedef struct Typetable {
    char *name;
    Fieldlist *fields;          
    int size;                   
    int isUser;                 
    struct Typetable *next;
} Typetable;

extern Typetable *TINT, *TSTR, *TBOOL, *TVOID;
extern Typetable *TTUPLE;     
void TypeTableCreate(void);                
Typetable *TLookup(const char *name);             
Typetable *TResolve(const char *name);            
Typetable *TInstall(const char *name);            
Fieldlist *FInstall(Typetable *t, const char *fname, Typetable *ftype);
Fieldlist *FLookup(Typetable *t, const char *fname);
int isUserType(Typetable *t);
int typeCompatible(Typetable *lhs, Typetable *rhs);   

void TypeTablePrint(void);
#define TYPE_NONE  ((struct Typetable *)0)
#define TYPE_BOOL  TBOOL
#define TYPE_INT   TINT
#define TYPE_STR   TSTR
#define TYPE_TUPLE TTUPLE

#endif