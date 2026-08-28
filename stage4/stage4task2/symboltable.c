#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "symboltable.h"

struct Gsymbol *Ghead=NULL;
int nextBinding=4096;

struct Gsymbol *Lookup(char *name){
    struct Gsymbol *temp=Ghead;
    while(temp!=NULL){
        if(strcmp(temp->name,name)==0){
            return temp;
        }
        temp=temp->next;
    }
    return NULL;
}
void Install(char *name,int type,int size){
    if(Lookup(name)!=NULL){
        printf("Error: Variable %s already declared\n",name);
        exit(1);
    }
    struct Gsymbol *temp=malloc(sizeof(struct Gsymbol));
    temp->name=strdup(name);
    temp->type=type;
    temp->size=size;
    temp->binding=nextBinding;
    temp->next=NULL;

    nextBinding+=size;
    if(Ghead==NULL){
        Ghead=temp;
    }
    else {
        struct Gsymbol *p=Ghead;
        while(p->next!=NULL){
            p=p->next;
        }
        p->next=temp;
    }
}
void PrintSymbolTable(){
    struct Gsymbol *temp=Ghead;
    printf("Name\tType\tSize\tBinding\n");
    while(temp!=NULL){
        printf("%s\t",temp->name);
        if(temp->type==TYPE_INT){
            printf("INT\t");
        }
        else if(temp->type==TYPE_STR){
            printf("STR\t");
        }
        printf("%d\t%d\n",temp->size,temp->binding);
        temp=temp->next;
    }
}