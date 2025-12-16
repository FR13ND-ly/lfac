
%code requires {
  #include <string>
  using namespace std;
}

%{
#include <stdio.h>
#include <stdlib.h>
#include "SymTable.h"

SymTable* current;
void yyerror(const char *s);

extern FILE* yyin;
extern char* yytext; //token current din lexer
extern int yylineno; //linia curenta
int errorCount = 0;
extern int yylex();
%}

%union {
    std::string* Str;
    int NR;
    float FLOAT_NR;
}

%destructor { delete $$; } <Str> 

%token ASSIGN SEMICOLON
%token<NR> NR
%token<FLOAT_NR> FLOAT_NR
%token<Str> ID TYPE
%start program







%%
program: declarations{if (errorCount == 0) cout<< "The program is correct!" << endl; };

declarations: decl 
            | declarations decl
            ;

decl: TYPE ID ';' {
        if(!current->existsId($2)){
            current->addVar($1, $2);
            delete $1;
            delete $2;
        }
        else{
            errorCount++;
            yyerror("Variable already defined");
        }
    }
    // | TYPE ID ASSIGN NR ';'{
    // if(!current->existsId($2)){
    //     current->addVar($1, $2);
    //     current->setValue($2, $4);    
    // } else {
    //     errorCount++;
    //     yyerror("Variable already defined");
    // }
    // }
;











%%
void yyerror(const char *s) {
    printf("Eroare de sintaxa: %s\n", s);
}


int main(int argc, char** argv) {

    yyin=fopen(argv[1],"r");

    current = new SymTable("global");
    yyparse();

    printf("Variables:\n");
    current->printVars();

    return 0;
}