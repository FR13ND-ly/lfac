#include "SymTable.h"

/* --- ADD THIS LINE --- */
int errorCount = 0; 

SymTable::SymTable(string tableName, SymTable* parent) 
    : parent(parent), name(tableName) {}

SymTable* SymTable::getParent() {
    return parent;
}

string SymTable::getName() {
    return name;
}

void SymTable::addVar(string type, string name) {
    IdInfo var(type, name, "var");
    ids[name] = var; 
}

void SymTable::addFunc(string type, string name) {
    IdInfo func(type, name, "func");
    ids[name] = func; 
}

void SymTable::addClass(string name) {
    IdInfo cls("class", name, "class");
    ids[name] = cls;
}

bool SymTable::existsId(string s) {
    if(ids.find(s) != ids.end()) return true;
    if(parent) return parent->existsId(s);
    return false;
}

bool SymTable::existsIdCurrentScope(string s) {
    return ids.find(s) != ids.end();
}

void SymTable::printVars() {
    cout << "Scope: " << name << endl;
    for (const auto& v : ids) {
        cout << "  Name: " << v.first << " | Type: " << v.second.type 
             << " | Cat: " << v.second.category << endl;
    }
    cout << "----------------" << endl;
}

SymTable::~SymTable() {
    ids.clear();
}