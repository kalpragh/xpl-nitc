%{
#include<stdio.h>
#include<string.h>
#include<ctype.h>
int level = 0;
%}
%token IF THEN ENDIF OTHER
%%
program : stmtlist
        ;
stmtlist : stmtlist stmt
         |
         ;
stmt : IF  { level++; printf("IF entered, nesting level = %d\n", level); }
       THEN stmtlist ENDIF  { printf("IF exited, nesting level = %d\n", level); level--; }
     | OTHER
     ;
%%
yyerror(char *s){ printf("%s\n", s); }

yylex()
{
    char word[100];
    int i = 0;
    char c;
    while((c=getchar())==' '||c=='\n'||c=='\t');
    if(c==EOF) return 0;
    word[i++] = c;
    while((c=getchar())!=' ' && c!='\n' && c!='\t' && c!=EOF)
        word[i++] = c;
    word[i] = '\0';
    if(strcmp(word,"if")==0) return IF;
    if(strcmp(word,"then")==0) return THEN;
    if(strcmp(word,"endif")==0) return ENDIF;
    return OTHER;
}

main(){ yyparse(); return 0; }