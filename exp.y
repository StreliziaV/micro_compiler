%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "added.h"
#include "exp.tab.h"

int yyerror (char *s);
int yylex();
FILE *yyin;
%}

%union {
    char id[32];
    int integer;
    struct expr_struct expr_st;
    struct expr_list_struct exprlist_st;
    struct primary_struct primary_st;
    struct id_list_struct idlist_st;
}
%start system_goal
%token BEGIN_ END READ WRITE
%token LPAREN RPAREN SEMICOLON COMMA
%token ASSIGNOP PLUOP MINUSOP SCANEOF
%token <integer> INTLITERAL
%token <id> ID
%type <idlist_st> id_list
%type <expr_st> expression
%type <exprlist_st> expr_list
%type <primary_st> primary

%%
system_goal     : program SCANEOF {;}
                ;

program         : BEGIN_ statement_list END{;}
                ;

statement_list  : statement {;}
                | statement_list statement {;}
                ;

statement       : ID ASSIGNOP expression SEMICOLON {assignment($1, &($3));}
                | READ LPAREN id_list RPAREN SEMICOLON {read_func(&($3));}
                | WRITE LPAREN expr_list RPAREN SEMICOLON {write_func(&($3));}
                ;

id_list         : ID {id_to_id_list($1, &($$));}
                | id_list COMMA ID {id_list_id_to_id_list(&($1), $3, &($$));}
                ;

expr_list       : expression {expr_to_expr_list(&($1), &($$));}
                | expr_list COMMA expression {expr_list_expr_to_expr_list(&($1), &($3), &($$));}
                ;

expression      : primary {primary_to_expression(&($1), &($$));}
                | expression PLUOP primary {expr_op_prim_to_expression(&($1), PLUOP, &($3), &($$));}
                | expression MINUSOP primary {expr_op_prim_to_expression(&($1), MINUSOP, &($3), &($$));}
                ;

primary         : LPAREN expression RPAREN {lp_expression_rp_to_primary(&($2), &($$));}
                | ID {id_to_primary($1, &($$));}
                | INTLITERAL {integer_to_primary($1, &($$));}
                | PLUOP primary {op_primary_to_primary(&($$), &($2), PLUOP);}
                | MINUSOP primary {op_primary_to_primary(&($$), &($2), MINUSOP);}
                ;

%%

int yyerror(char *s) {
    printf("wrong syntax on line %s\n", s);
    return 0;
}

int main(int argc, char const *argv[]) {
    remove("output.asm");
    yyin = fopen(argv[1], "r");
    yyparse();

    char my_line[50];
    FILE *output;
    if ((output = fopen("output.asm", "r")) == NULL) {
        printf("compilation failed, please check your input");
        return 0;
    }
    while (1) {
        if (fgets(my_line, 50, output) == NULL) break;
        printf("%s", my_line);
    }
    fclose(output);
    printf("\n");

    printf("compilation finished, the generated code has been written in file output.asm\n");
    return 0;
}