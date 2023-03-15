%{
#include <stdio.h>
#include <string.h>
#include "added.h"
#include "exp.tab.h"
int yyerror (char *s);
int yylex();
int end = 0;
%}

integer ([0-9])+
id ([A-Z0-9]){1,32}
comment --(.)*\n

%%
"begin" {return BEGIN_;}
"end" {return END;}
"read" {return READ;}
"write" {return WRITE;}
{integer} {yylval.integer = atoi(yytext); return INTLITERAL;}
{id} {sscanf(yytext, "%s", yylval.id); return ID;}
{comment} ;
"(" {return LPAREN;}
")" {return RPAREN;}
";" {return SEMICOLON;}
"," {return COMMA;}
":=" {return ASSIGNOP;}
"+" {return PLUOP;}
"-" {return MINUSOP;}
[ \t\n] ;

<<EOF>> {if (!end) {end = 1; return SCANEOF;} yyterminate();}
%%


int yywrap (void) {return 1;}