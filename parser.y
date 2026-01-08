%{
    #include "AST.h"
    #include <vector>
    #include <string>
    #include <fstream>
    #include <iostream>
    #include <utility>
    
    extern int yylex();
    extern int yylineno;
    extern FILE* yyin;
    void yyerror(const char *s);

    SymbolTable* globalScope = new SymbolTable("Global");
    SymbolTable* currentScope = globalScope;
    vector<SymbolTable*> allScopes; 
    vector<ASTNode*> mainProgramAST; 

    string currentClassName = "";
    
    vector<pair<string, DataType>> tempParamsDef; 
%}

%code requires {
    #include "AST.h"
}

%union {
    int intVal;
    float floatVal;
    bool boolVal;
    std::string* strVal;
    DataType dataType;
    ASTNode* node;
    std::vector<ASTNode*>* nodeVec; 
}

%token <intVal> INT_VAL
%token <floatVal> FLOAT_VAL
%token <boolVal> BOOL_VAL
%token <strVal> STRING_VAL ID
%token <dataType> T_INT T_FLOAT T_STRING T_BOOL T_VOID

%token LET CLASS FUNCTION MAIN IF ELSE WHILE PRINT NEW RETURN
%token ASSIGN EQ AND OR NOT

%type <dataType> type
%type <node> expression statement assign_stmt print_stmt if_stmt while_stmt call_expr return_stmt
%type <nodeVec> statement_list arg_list args_exist func_body

%left OR
%left AND
%left EQ '>' '<'
%left '+' '-'
%left '*' '/'
%right NOT

%%

program: definitions main_block 
       ;

definitions:
           | definitions var_decl
           | definitions class_def
           | definitions func_def
           ;

var_decl: LET ID ':' type ';' {
            if (currentScope->symbols.count(*$2)) {
                yyerror("Variable already defined");
            } else {
                SymbolInfo* s = new SymbolInfo(*$2, "variable", $4);
                currentScope->add(s);
            }
            delete $2;
        }
        | LET ID ':' type ASSIGN expression ';' {
             if (currentScope->symbols.count(*$2)) yyerror("Variable already defined");
             if ($4 != $6->nodeType) yyerror("Semantic Error: Type mismatch in initialization");
             SymbolInfo* s = new SymbolInfo(*$2, "variable", $4);
             currentScope->add(s);
             delete $2; delete $6; 
        }
        | LET ID ':' ID ';' { 
             if (!globalScope->lookup(*$4)) yyerror("Undefined class");
             SymbolInfo* s = new SymbolInfo(*$2, "variable", TYPE_CLASS);
             s->className = *$4;
             currentScope->add(s);
             delete $2; delete $4;
        }
        ;

class_def: CLASS ID { 
             if (globalScope->symbols.count(*$2)) yyerror("Class name already exists");
             
             SymbolInfo* s = new SymbolInfo(*$2, "class", TYPE_CLASS);
             globalScope->add(s);
             
             currentClassName = *$2;
             SymbolTable* classScope = new SymbolTable("Class_" + *$2, globalScope);
             allScopes.push_back(classScope);
             currentScope = classScope;
             
           } '{' class_body '}' {
             SymbolInfo* cls = globalScope->lookup(currentClassName);
             for(auto const& entry : currentScope->symbols) {
                 if(entry.second->kind == "field") {
                     cls->classMembers[entry.first] = entry.second->type;
                 }
             }
             cls->funcScopeRef = currentScope; 

             if(currentScope->parent) currentScope = currentScope->parent;
             currentClassName = "";
             delete $2;
           }
         ;

class_body:
          | class_body field_decl
          | class_body method_def
          ;

field_decl: LET ID ':' type ';' {
              if (currentScope->symbols.count(*$2)) yyerror("Field already defined");
              SymbolInfo* s = new SymbolInfo(*$2, "field", $4);
              currentScope->add(s);
              delete $2;
          }
          ;

func_def: FUNCTION ID { tempParamsDef.clear(); } '(' param_list ')' ':' type {
             SymbolInfo* s = new SymbolInfo(*$2, "function", $8);
             
             for(auto p : tempParamsDef) {
                 s->paramTypes.push_back(p.second);
                 s->paramNames.push_back(p.first);
             }
             currentScope->add(s);
             
             SymbolTable* funcScope = new SymbolTable("Func_" + *$2, currentScope);
             allScopes.push_back(funcScope);
             s->funcScopeRef = funcScope; 
             currentScope = funcScope;

             for(auto p : tempParamsDef) {
                 SymbolInfo* paramSym = new SymbolInfo(p.first, "parameter", p.second);
                 currentScope->add(paramSym);
             }

          } '{' func_body '}' {
             SymbolInfo* funcSym = currentScope->parent->lookup(*$2);
             if (funcSym) {
                 funcSym->funcBody = *$11; 
             }
             if(currentScope->parent) currentScope = currentScope->parent;
             delete $2;
             delete $11;
          }
        ;

method_def: FUNCTION ID { tempParamsDef.clear(); } '(' param_list ')' ':' type {
             SymbolInfo* s = new SymbolInfo(*$2, "function", $8);
             for(auto p : tempParamsDef) {
                 s->paramTypes.push_back(p.second);
                 s->paramNames.push_back(p.first);
             }
             currentScope->add(s);
             
             SymbolTable* funcScope = new SymbolTable("Method_" + *$2, currentScope);
             allScopes.push_back(funcScope);
             s->funcScopeRef = funcScope;
             currentScope = funcScope;

             for(auto p : tempParamsDef) {
                 SymbolInfo* paramSym = new SymbolInfo(p.first, "parameter", p.second);
                 currentScope->add(paramSym);
             }
          } '{' func_body '}' {
             SymbolInfo* funcSym = currentScope->parent->lookup(*$2);
             if (funcSym) {
                 funcSym->funcBody = *$11;
             }
             if(currentScope->parent) currentScope = currentScope->parent;
             delete $2;
             delete $11;
          }
        ;

param_list:
          | param_decl
          | param_list ',' param_decl
          ;

param_decl: ID ':' type {
             tempParamsDef.push_back({*$1, $3});
             delete $1;
          }
          ;

func_body: local_decls statement_list { $$ = $2; }
         ;

local_decls:
           | local_decls var_decl
           ;

main_block: MAIN { 
                allScopes.push_back(globalScope); 
            } '{' statement_list '}' {
              mainProgramAST = *$4;
              delete $4;
          }
          ;

statement_list: { $$ = new vector<ASTNode*>(); }
              | statement_list statement {
                  if ($2 != NULL) {
                      $1->push_back($2);
                  }
                  $$ = $1;
              }
              ;

statement: assign_stmt { $$ = $1; }
         | print_stmt { $$ = $1; }
         | if_stmt { $$ = $1; }   
         | while_stmt { $$ = $1; } 
         | call_expr ';' { $$ = $1; }
         | return_stmt { $$ = $1; }
         ;

assign_stmt: ID ASSIGN expression ';' {
               SymbolInfo* s = currentScope->lookup(*$1);
               if(!s) yyerror("Undefined variable");
               if (s->type != $3->nodeType) yyerror("Semantic Error: Assignment type mismatch");
               $$ = new AssignNode(*$1, currentScope, $3);
               delete $1;
           }
           | ID '.' ID ASSIGN expression ';' {
               SymbolInfo* s = currentScope->lookup(*$1);
               if(!s) yyerror("Undefined object");
               if(s->type != TYPE_CLASS) yyerror("Variable is not an object");
               
               SymbolInfo* cls = globalScope->lookup(s->className);
               if(!cls || cls->classMembers.count(*$3) == 0) yyerror("Member not found");
               
               DataType fieldType = cls->classMembers[*$3];
               if(fieldType != $5->nodeType) yyerror("Semantic Error: Field assignment type mismatch");

               $$ = new AssignNode(*$1, *$3, currentScope, $5);
               delete $1; delete $3;
           }
           ;

print_stmt: PRINT '(' expression ')' ';' {
              $$ = new PrintNode($3);
          }
          ;

if_stmt: IF '(' expression ')' '{' statement_list '}' {
           if ($3->nodeType != TYPE_BOOL) yyerror("IF condition must be boolean");
           $$ = new IfNode($3, *$6);
           delete $6;
       }
       | IF '(' expression ')' '{' statement_list '}' ELSE '{' statement_list '}' {
           if ($3->nodeType != TYPE_BOOL) yyerror("IF condition must be boolean");
           // Asigura-te ca in AST.h IfNode accepta 2 vectori (then, else)
           $$ = new IfNode($3, *$6, *$10); 
           delete $6; delete $10;
       }
       ;

while_stmt: WHILE '(' expression ')' '{' statement_list '}' {
            if ($3->nodeType != TYPE_BOOL) yyerror("WHILE condition must be boolean");
            $$ = new WhileNode($3, *$6);
            delete $6;
          }
          ;

return_stmt: RETURN expression ';' {
                $$ = new ReturnNode($2);
            }
            ;

call_expr: ID '(' arg_list ')' {
             SymbolInfo* s = currentScope->lookup(*$1);
             if (!s || s->kind != "function") yyerror("Function not defined");
             
             if (s->paramTypes.size() != $3->size()) yyerror("Wrong number of arguments");
             
             for(size_t i=0; i < $3->size(); i++) {
                 if ((*$3)[i]->nodeType != s->paramTypes[i]) 
                    yyerror("Argument type mismatch");
             }

             $$ = new FunctionCallNode(*$1, *$3, currentScope, s->type);
             delete $1;
             delete $3; 
         }
         | ID '.' ID '(' arg_list ')' {
             SymbolInfo* obj = currentScope->lookup(*$1);
             if(!obj || obj->type != TYPE_CLASS) yyerror("Not an object");
             
             SymbolInfo* cls = globalScope->lookup(obj->className);
             if(!cls) yyerror("Class definition not found");

             SymbolTable* clsScope = cls->funcScopeRef;
             if(!clsScope) yyerror("Class scope not found");

             SymbolInfo* method = clsScope->lookup(*$3);
             if(!method || method->kind != "function") yyerror("Method not found");

             if (method->paramTypes.size() != $5->size()) yyerror("Wrong number of arguments in method call");

             $$ = new RealMethodCallNode(*$1, *$5, method, currentScope);
             delete $1; delete $3; 
         }
         ;

arg_list: { $$ = new vector<ASTNode*>(); }
        | args_exist { $$ = $1; }
        ;

args_exist: expression { 
              $$ = new vector<ASTNode*>(); 
              $$->push_back($1); 
            }
          | args_exist ',' expression {
              $1->push_back($3);
              $$ = $1;
          }
          ;

expression: INT_VAL { $$ = new LiteralNode($1); }
          | FLOAT_VAL { $$ = new LiteralNode($1); }
          | STRING_VAL { $$ = new LiteralNode(*$1); delete $1; }
          | BOOL_VAL { $$ = new LiteralNode($1); }
          | ID { 
              SymbolInfo* s = currentScope->lookup(*$1);
              if (!s) yyerror("Undefined variable");
              $$ = new IdNode(*$1, currentScope); delete $1; 
          }
          | ID '.' ID {
              SymbolInfo* s = currentScope->lookup(*$1);
              if (!s) yyerror("Undefined object");
              if (s->type != TYPE_CLASS) yyerror("Not an object");
              SymbolInfo* cls = globalScope->lookup(s->className);
              if(!cls || cls->classMembers.count(*$3) == 0) yyerror("Field not found");
              DataType t = cls->classMembers[*$3];
              $$ = new MemberAccessNode(*$1, *$3, currentScope, t);
              delete $1; delete $3;
          }
          | call_expr { $$ = $1; }
          | expression '+' expression { 
              if ($1->nodeType != $3->nodeType) yyerror("Type mismatch in +");
              $$ = new BinaryNode($1, "+", $3); 
          }
          | expression '-' expression { 
              if ($1->nodeType != $3->nodeType) yyerror("Type mismatch in -");
              $$ = new BinaryNode($1, "-", $3); 
          }
          | expression '*' expression { 
              if ($1->nodeType != $3->nodeType) yyerror("Type mismatch in *");
              $$ = new BinaryNode($1, "*", $3); 
          }
          | expression '/' expression { 
              if ($1->nodeType != $3->nodeType) yyerror("Type mismatch in /");
              $$ = new BinaryNode($1, "/", $3); 
          }
          | expression '>' expression { 
              if ($1->nodeType != $3->nodeType) yyerror("Type mismatch in >");
              $$ = new BinaryNode($1, ">", $3); 
          }
          | expression '<' expression { 
              if ($1->nodeType != $3->nodeType) yyerror("Type mismatch in <");
              $$ = new BinaryNode($1, "<", $3); 
          }
          | expression EQ expression {
              if ($1->nodeType != $3->nodeType) yyerror("Type mismatch in ==");
              $$ = new BinaryNode($1, "==", $3);
          }
          | expression AND expression {
              if ($1->nodeType != TYPE_BOOL || $3->nodeType != TYPE_BOOL) 
                  yyerror("Logic operator && requires boolean operands");
              $$ = new BinaryNode($1, "&&", $3);
          }
          | expression OR expression {
              if ($1->nodeType != TYPE_BOOL || $3->nodeType != TYPE_BOOL) 
                  yyerror("Logic operator || requires boolean operands");
              $$ = new BinaryNode($1, "||", $3);
          }
          | NOT expression {
              if ($2->nodeType != TYPE_BOOL) 
                  yyerror("Logic operator ! requires boolean operand");
              $$ = new UnaryNode("!", $2);
          }
          | NEW ID '(' ')' {
               if (!globalScope->lookup(*$2)) yyerror("Undefined class");
               $$ = new NewNode(*$2, globalScope);
               delete $2;
          }
          | '(' expression ')' { $$ = $2; }
          ;

type: T_INT { $$ = TYPE_INT; }
    | T_FLOAT { $$ = TYPE_FLOAT; }
    | T_STRING { $$ = TYPE_STRING; }
    | T_BOOL { $$ = TYPE_BOOL; }
    | T_VOID { $$ = TYPE_VOID; }
    ;

%%

void yyerror(const char *s) {
    cout << "Error: " << s << " at line " << yylineno << endl;
    exit(1);
}

int main(int argc, char** argv) {
    if (argc > 1) yyin = fopen(argv[1], "r");
    else yyin = stdin;
    
    yyparse();

    ofstream f("tables.txt");
    for(SymbolTable* t : allScopes) {
        t->printTable(f);
        f << endl;
    }
    f.close();

    cout << "Res:" << endl;
    for(ASTNode* node : mainProgramAST) {
        if(node != NULL) {
            node->eval();
        }
    }

    return 0;
}