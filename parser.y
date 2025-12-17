%{
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include "SymTable.h"

extern int yylex();
extern int yylineno;
extern char* yytext;
void yyerror(const char *s);

SymTable* current;
extern int errorCount; 

void enterScope(string name) {
    current = new SymTable(name, current);
}

void exitScope() {
    SymTable* temp = current;
    current = current->getParent();
    delete temp;
}
%}

%code requires {
  #include <string>
  using namespace std;
}

%union {
    std::string* Str;
    int NR;
    float FLOAT_NR;
}

%token ASSIGN SEMICOLON COMMA DOT
%token PLUS MINUS MUL DIV AND OR NOT EQ NEQ LT GT
%token LPAREN RPAREN LBRACE RBRACE
%token CLASS IF ELSE WHILE MAIN RETURN PRINT NEW
%token<NR> NR
%token<FLOAT_NR> FLOAT_NR
%token<Str> ID TYPE BOOL_VAL STRING_VAL

%left OR
%left AND
%left EQ NEQ
%left LT GT
%left PLUS MINUS
%left MUL DIV
%right NOT
%right ASSIGN 

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%start program

%%

program: global_declarations main_block 
       { 
           if (errorCount == 0) cout << ">> Program parsed successfully!" << endl; 
           if (current) current->printVars(); 
       }
       ;

global_declarations: /* empty */
                   | global_declarations global_decl
                   ;

global_decl: class_decl
           | func_decl
           | var_decl
           ;

var_decl: TYPE ID SEMICOLON {
            if(current->existsIdCurrentScope(*$2)){
                yyerror(("Variable '" + *$2 + "' already defined").c_str());
                errorCount++;
            } else {
                current->addVar(*$1, *$2);
            }
            delete $1; delete $2;
        }
        | ID ID SEMICOLON { 
             if(current->existsIdCurrentScope(*$2)){
                yyerror(("Variable '" + *$2 + "' already defined").c_str());
                errorCount++;
            } else {
                current->addVar(*$1, *$2);
            }
            delete $1; delete $2;
        }
        ;

class_decl: CLASS ID {
                if(current->existsId(*$2)) {
                    yyerror("Class name already exists"); errorCount++;
                } else {
                    current->addClass(*$2);
                }
                enterScope(*$2); 
            } 
            LBRACE class_body RBRACE {
                exitScope(); 
                delete $2;
            }
          ;

class_body: /* empty */
          | class_body class_member
          ;

class_member: var_decl 
            | func_decl 
            ;

func_decl: TYPE ID {
               if(current->existsIdCurrentScope(*$2)) {
                   yyerror("Function already defined"); errorCount++;
               } else {
                   current->addFunc(*$1, *$2);
               }
               enterScope(*$2); 
           }
           LPAREN func_params RPAREN LBRACE func_body RBRACE {
               exitScope(); 
               delete $1; delete $2;
           }
         ;

func_params: /* empty */
           | param_list
           ;

param_list: TYPE ID { current->addVar(*$1, *$2); delete $1; delete $2; }
          | param_list COMMA TYPE ID { current->addVar(*$3, *$4); delete $3; delete $4; }
          ;

/* FIXED: Allow mixed statements and declarations to resolve ID ambiguity */
func_body: func_stmt_list
         ;

func_stmt_list: /* empty */
              | func_stmt_list func_element
              ;

func_element: var_decl
            | statement
            ;

main_block: MAIN { enterScope("main"); } 
            LBRACE statement_list RBRACE { 
                exitScope(); 
            }
          ;

/* Standard Block (for if/while) - No declarations allowed per requirements */
block: LBRACE statement_list RBRACE 
     ;

statement_list: /* empty */
              | statement_list statement
              ;

statement: assignment
         | if_stmt
         | while_stmt
         | func_call SEMICOLON
         | print_stmt
         | RETURN expr SEMICOLON
         ;

assignment: ID ASSIGN expr SEMICOLON
          | ID DOT ID ASSIGN expr SEMICOLON 
          | ID ASSIGN NEW ID LPAREN RPAREN SEMICOLON 
          ;

print_stmt: PRINT LPAREN expr RPAREN SEMICOLON
          ;

if_stmt: IF LPAREN bool_expr RPAREN block %prec LOWER_THAN_ELSE
       | IF LPAREN bool_expr RPAREN block ELSE block
       ;

while_stmt: WHILE LPAREN bool_expr RPAREN block
          ;

func_call: ID LPAREN args RPAREN
         | ID DOT ID LPAREN args RPAREN 
         ;

args: /* empty */
    | args_list
    ;

args_list: expr
         | args_list COMMA expr
         ;

expr: arithmetic_expr
    | bool_expr
    | STRING_VAL
    | func_call 
    ;

arithmetic_expr: term
               | arithmetic_expr PLUS term
               | arithmetic_expr MINUS term
               ;

term: factor
    | term MUL factor
    | term DIV factor
    ;

factor: ID
      | ID DOT ID 
      | NR
      | FLOAT_NR
      | LPAREN arithmetic_expr RPAREN
      ;

bool_expr: BOOL_VAL
         | arithmetic_expr EQ arithmetic_expr
         | arithmetic_expr NEQ arithmetic_expr
         | arithmetic_expr LT arithmetic_expr
         | arithmetic_expr GT arithmetic_expr
         | bool_expr AND bool_expr
         | bool_expr OR bool_expr
         | NOT bool_expr
         | LPAREN bool_expr RPAREN
         ;

%%

void yyerror(const char *s) {
    printf("Syntax Error on line %d: %s\n", yylineno, s);
}