%{
	#include<stdio.h>
	int switched = 0;
%}
%%
[0-9]+ 	printf("number: %s\n", yytext);
%%
int yywrap()
{
	if(!switched)
	{
		switched = 1;
		yyin = fopen("input_file.l", "r");
		printf("--- switching to input_file.l ---\n");
		return 0;
	}
	else
		return 1;
}
int main()
{
	yylex();
	return 1;
}
