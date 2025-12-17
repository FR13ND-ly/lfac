#pragma once
#include <iostream>
#include <map>
#include <string>
#include <vector>

using namespace std;

class IdInfo {
public:
    string type;
    string name;
    string category; 

    IdInfo() {}
    IdInfo(string type, string name, string category = "var") 
        : type(type), name(name), category(category) {}
};

class SymTable {
    SymTable* parent;
    map<string, IdInfo> ids;
    string name;

public:
    SymTable(string tableName, SymTable* parent = NULL);
    
    bool existsId(string s);
    bool existsIdCurrentScope(string s);
    
    void addVar(string type, string name);
    void addFunc(string type, string name);
    void addClass(string name);
    
    SymTable* getParent();
    string getName();
    void printVars();
    ~SymTable();
};

/* --- ADD THIS LINE --- */
extern int errorCount;