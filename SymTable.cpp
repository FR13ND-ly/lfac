#include "SymTable.h"
using namespace std;

void SymTable::addVar(string* type, string*name) {
    IdInfo var(type, name);
    ids[*name] = var; 
}

bool SymTable::existsId(string* s) {
    if(ids.find(*s) != ids.end()) return true;
    if(parent) return parent->existsId(s);
    return false;
}


void SymTable::printVars() {
    for (const pair<string, IdInfo>& v : ids) {
        cout << "name: " << v.first << " type:" << v.second.type << endl; 
     }
}

void SymTable::setValue(string* name, int val) {
    auto it = ids.find(*name);
    if(it != ids.end() && it->second.type == "int") {
        it->second.intValue = val;
    }
}

void SymTable::setValue(string* name, float val) {
    auto it = ids.find(*name);
    if(it != ids.end() && it->second.type == "float")
        it->second.floatValue = val;
}

void SymTable::setValue(string* name, bool val) {
    auto it = ids.find(*name);
    if(it != ids.end() && it->second.type == "bool")
        it->second.boolValue = val;
}

void SymTable::setValue(string* name, string val) {
    auto it = ids.find(*name);
    if(it != ids.end() && it->second.type == "string")
        it->second.strValue = val;
}


SymTable::~SymTable() {
    ids.clear();
}











