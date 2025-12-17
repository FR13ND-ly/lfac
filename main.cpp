#include <iostream>
#include <stdio.h>
#include "SymTable.h"

using namespace std;

extern int yyparse();
extern FILE* yyin;
extern int yylineno;
extern SymTable* current;

int main(int argc, char** argv) {
    if (argc < 2) {
        cout << "Usage: " << argv[0] << " <input_file>" << endl;
        return 1;
    }

    yyin = fopen(argv[1], "r");
    if (!yyin) {
        cout << "Error: Cannot open file " << argv[1] << endl;
        return 1;
    }

    current = new SymTable("global");

    int result = yyparse();

    if (current) {
        delete current;
    }
    
    fclose(yyin);

    if (result == 0) {
        cout << "Compilation finished successfully." << endl;
    }

    return result;
}