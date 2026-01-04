%{
    #include "AST.h"
    #include <vector>
    #include <string>
    
    extern int yylex();
    extern int yylineno;
    void yyerror(const char *s);

    SymbolTable* globalScope = new SymbolTable("Global");
    SymbolTable* currentScope = globalScope;
    vector<SymbolTable*> allScopes = {globalScope};
    vector<ASTNode*> mainProgramAST;

    // Structuri temporare pentru construirea funcțiilor
    vector<DataType> currentParamsTypes; 
    
    void enterScope(string name) {
        SymbolTable* ns = new SymbolTable(name, currentScope);
        allScopes.push_back(ns);
        currentScope = ns;
    }
    void exitScope() {
        if(currentScope->parent) currentScope = currentScope->parent;
    }
%}

%code requires {
    #include "AST.h"
    #include <vector>
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

%token LET CLASS FUNCTION MAIN IF ELSE WHILE RETURN PRINT COLON
%token EQ AND OR

/* Tipuri pentru noduri AST */
%type <node> expression term factor statement assign_stmt print_stmt function_call if_stmt while_stmt
%type <dataType> type
%type <nodeVec> call_arg_list

%left OR
%left AND
%left EQ
%left '<' '>'
%left '+' '-'
%left '*' '/'
%left '.'

%%

program: global_zone main_zone ;

global_zone: global_zone global_element | /* empty */ ;

global_element: class_definition | function_definition | variable_declaration ;

/* --- DECLARAȚII VARIABILE --- */
variable_declaration: LET ID COLON type ';' {
    if (!currentScope->add(new SymbolInfo(*$2, "variable", $4))) 
        yyerror("Semantic Error: Variable already defined.");
    
    if (currentScope->parent && globalScope->lookup(currentScope->scopeName) && globalScope->lookup(currentScope->scopeName)->kind == "class") {
        SymbolInfo* cls = globalScope->lookup(currentScope->scopeName);
        cls->members[*$2] = $4;
    }
    delete $2;
}
| LET ID COLON type '=' expression ';' {
    if ($6->nodeType != $4 && $6->nodeType != TYPE_UNKNOWN) 
        yyerror("Semantic Error: Type mismatch in initialization.");
    
    SymbolInfo* s = new SymbolInfo(*$2, "variable", $4);
    s->runtimeValue = $6->eval(); 
    currentScope->add(s);
    delete $2;
}
| LET ID COLON ID ';' { 
    SymbolInfo* cls = globalScope->lookup(*$4);
    if (!cls || cls->kind != "class") yyerror("Semantic Error: Unknown class type.");
    SymbolInfo* obj = new SymbolInfo(*$2, "variable", TYPE_CLASS);
    obj->className = *$4;
    currentScope->add(obj);
    delete $2; delete $4;
};

/* --- DEFINIȚII CLASE --- */
class_definition: CLASS ID {
    globalScope->add(new SymbolInfo(*$2, "class", TYPE_CLASS));
    enterScope(*$2);
} '{' class_members '}' {
    exitScope();
    delete $2;
};

class_members: class_members class_member | /* empty */ ;

class_member: variable_declaration | function_definition ;

/* --- DEFINIȚII FUNCȚII --- */
function_definition: FUNCTION ID { currentParamsTypes.clear(); } '(' parameter_list ')' COLON type '{' {
    SymbolInfo* func = new SymbolInfo(*$2, "function", $8);
    func->parameterTypes = currentParamsTypes;
    currentScope->add(func);
    enterScope(*$2);
} function_body '}' {
    exitScope();
    delete $2;
};

parameter_list: params | /* empty */ ;
params: params ',' param | param ;
param: ID COLON type { 
    currentParamsTypes.push_back($3);
    currentScope->add(new SymbolInfo(*$1, "parameter", $3)); 
    delete $1; 
};

function_body: declarations_zone statements_zone ;
declarations_zone: declarations_zone variable_declaration | /* empty */ ;
statements_zone: statements_zone statement | /* empty */ ;

/* --- MAIN --- */
main_zone: MAIN '{' main_statements '}' {
    cout << "--- EVALUATING MAIN BLOCK ---" << endl;
    for(auto node : mainProgramAST) {
        if(node) node->eval();
    }
};

main_statements: main_statements main_entry | /* empty */ ;
main_entry: statement { if($1) mainProgramAST.push_back($1); } 
          | variable_declaration { /* Permitem declaratii in main pentru testare */ } ;

/* --- STATEMENTS --- */
statement: assign_stmt | print_stmt | if_stmt | while_stmt | function_call ';' { $$ = $1; } ;

assign_stmt: ID '=' expression ';' {
    SymbolInfo* s = currentScope->lookup(*$1);
    if (!s) yyerror("Semantic Error: Variable not defined.");
    if (s->type != $3->nodeType && $3->nodeType != TYPE_UNKNOWN) 
        yyerror("Semantic Error: Type mismatch in assignment.");
    $$ = new AssignNode(*$1, currentScope, $3);
    delete $1;
}
| ID '.' ID '=' expression ';' {
    SymbolInfo* obj = currentScope->lookup(*$1);
    if (!obj || obj->type != TYPE_CLASS) yyerror("Semantic Error: Left side is not an object.");
    
    SymbolInfo* cls = globalScope->lookup(obj->className);
    if (cls->members.find(*$3) == cls->members.end()) yyerror("Semantic Error: Class member not found.");
    
    if (cls->members[*$3] != $5->nodeType) yyerror("Semantic Error: Type mismatch in field assignment.");
    
    $$ = new AssignNode(*$1, currentScope, $5); 
    delete $1; delete $3;
};

print_stmt: PRINT '(' expression ')' ';' { $$ = new PrintNode($3); };

if_stmt: IF '(' expression ')' '{' statements_zone '}' { $$ = nullptr; } ;
while_stmt: WHILE '(' expression ')' '{' statements_zone '}' { $$ = nullptr; } ;

function_call: ID '(' call_arg_list ')' {
    SymbolInfo* func = globalScope->lookup(*$1);
    if (!func || func->kind != "function") yyerror("Semantic Error: Function not defined.");
    
    if (func->parameterTypes.size() != $3->size()) 
        yyerror("Semantic Error: Incorrect number of arguments in function call.");
    
    for(size_t i=0; i < func->parameterTypes.size(); ++i) {
        if ((*$3)[i]->nodeType != func->parameterTypes[i] && (*$3)[i]->nodeType != TYPE_UNKNOWN)
             yyerror("Semantic Error: Argument type mismatch.");
    }

    $$ = new CallNode(*$1, *$3, func->type);
    delete $1; delete $3;
};

call_arg_list: call_arg_list ',' expression {
    $1->push_back($3);
    $$ = $1;
}
| expression {
    $$ = new std::vector<ASTNode*>();
    $$->push_back($1);
}
| /* empty */ {
    $$ = new std::vector<ASTNode*>();
};

/* --- EXPRESSIONS --- */
expression: expression '+' term { 
              if ($1->nodeType != $3->nodeType) yyerror("Semantic Error: Type mismatch (+).");
              $$ = new BinaryNode($1, "+", $3); 
          }
          | expression EQ term  { 
              if ($1->nodeType != $3->nodeType) yyerror("Semantic Error: Type mismatch (==).");
              $$ = new BinaryNode($1, "==", $3); 
          }
          | expression '>' term {
              if ($1->nodeType != $3->nodeType) yyerror("Semantic Error: Type mismatch (>).");
              $$ = new BinaryNode($1, ">", $3);
          }
          | term { $$ = $1; } 
          ;

term: term '*' factor { 
        if ($1->nodeType != $3->nodeType) yyerror("Semantic Error: Type mismatch (*).");
        $$ = new BinaryNode($1, "*", $3); 
      }
    | factor { $$ = $1; } 
    ;

factor: INT_VAL { $$ = new LiteralNode($1); }
      | FLOAT_VAL { $$ = new LiteralNode($1); }
      | STRING_VAL { $$ = new LiteralNode(*$1); delete $1; }
      | BOOL_VAL { $$ = new LiteralNode($1); }
      | ID {
          SymbolInfo* s = currentScope->lookup(*$1);
          if(!s) yyerror("Semantic Error: Identifier not found.");
          $$ = new IdNode(*$1, currentScope);
          delete $1;
      }
      | ID '.' ID {
          SymbolInfo* obj = currentScope->lookup(*$1);
          if (!obj || obj->type != TYPE_CLASS) yyerror("Semantic Error: Accessing member of non-object.");
          SymbolInfo* cls = globalScope->lookup(obj->className);
          if (cls->members.find(*$3) == cls->members.end()) yyerror("Semantic Error: Member not found.");
          
          $$ = new MemberAccessNode(*$1, *$3, currentScope, cls->members[*$3]);
          delete $1; delete $3;
      }
      | function_call { $$ = $1; }
      | '(' expression ')' { $$ = $2; }
      ;

type: T_INT { $$ = TYPE_INT; } | T_FLOAT { $$ = TYPE_FLOAT; } 
    | T_STRING { $$ = TYPE_STRING; } | T_BOOL { $$ = TYPE_BOOL; } | T_VOID { $$ = TYPE_VOID; } 
    ;

%%

void yyerror(const char *s) {
    cerr << "[COMPILATION ERROR] Line " << yylineno << ": " << s << endl;
    exit(1);
}

int main(int argc, char** argv) {
    extern FILE* yyin;
    if(argc > 1) yyin = fopen(argv[1], "r");
    
    yyparse();
    
    ofstream file("tables.txt");
    for(auto scope : allScopes) {
        scope->printTable(file);
    }
    file.close();
    
    cout << "Compilation successful. Symbol tables written to tables.txt" << endl;
    return 0;
}