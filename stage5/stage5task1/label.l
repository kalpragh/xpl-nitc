%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LABELS 1000

typedef struct {
    char name[64];
    int addr;
} Label;

Label table[MAX_LABELS];
int labelCount = 0;
int address = 2056;
int pass = 1;
int headerCount = 0;
FILE *fout;

void addLabel(char *name, int addr)
{
    strcpy(table[labelCount].name, name);
    table[labelCount].addr = addr;
    labelCount++;
}

int lookupLabel(char *name)
{
    for(int i = 0; i < labelCount; i++)
    {
        if(strcmp(table[i].name, name) == 0)
            return table[i].addr;
    }

    fprintf(stderr, "Undefined label: %s\n", name);
    exit(1);
}
%}

LETTER [A-Za-z_]
DIGIT [0-9]
ID {LETTER}({LETTER}|{DIGIT})*

%%

^{ID}":"[ \t]*\n {
    char name[64];
    sscanf(yytext, "%[^:]", name);

    if(pass == 1)
        addLabel(name, address);
}

^"JMP"[ \t]+{ID}[ \t]*\n {
    char label[64];
    sscanf(yytext, "JMP %s", label);

    if(pass == 1)
        address += 2;
    else {
        fprintf(fout, "JMP %d\n", lookupLabel(label));
        address += 2;
    }
}

^"JZ"[ \t]+"R"{DIGIT}+[ \t]*,[ \t]*{ID}[ \t]*\n {
    int r;
    char label[64];

    sscanf(yytext, "JZ R%d, %s", &r, label);

    if(pass == 1)
        address += 2;
    else {
        fprintf(fout, "JZ R%d, %d\n", r, lookupLabel(label));
        address += 2;
    }
}

^"JNZ"[ \t]+"R"{DIGIT}+[ \t]*,[ \t]*{ID}[ \t]*\n {
    int r;
    char label[64];

    sscanf(yytext, "JNZ R%d, %s", &r, label);

    if(pass == 1)
        address += 2;
    else {
        fprintf(fout, "JNZ R%d, %d\n", r, lookupLabel(label));
        address += 2;
    }
}

^.*\n {
    if(headerCount < 8) {
        headerCount++;

        if(pass == 2)
            fprintf(fout, "%s", yytext);
    }
    else {
        if(pass == 1)
            address += 2;
        else {
            fprintf(fout, "%s", yytext);
            address += 2;
        }
    }
}

%%

int yywrap()
{
    return 1;
}

int main(int argc, char **argv)
{
    if(argc != 3) {
        printf("Usage: %s input.xsm output.xsm\n", argv[0]);
        return 1;
    }

    yyin = fopen(argv[1], "r");

    if(yyin == NULL) {
        perror("input file");
        return 1;
    }

    pass = 1;
    address = 2056;
    headerCount = 0;
    labelCount = 0;

    yylex();

    fclose(yyin);

    yyin = fopen(argv[1], "r");
    fout = fopen(argv[2], "w");

    if(yyin == NULL || fout == NULL) {
        perror("file");
        return 1;
    }

    pass = 2;
    address = 2056;
    headerCount = 0;

    yyrestart(yyin);

    yylex();

    fclose(yyin);
    fclose(fout);

    printf("Label translation complete.\n");

    return 0;
}