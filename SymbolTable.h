#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <fstream>

using namespace std;

class ASTNode; 

enum DataType { TYPE_INT, TYPE_FLOAT, TYPE_STRING, TYPE_BOOL, TYPE_VOID, TYPE_CLASS, TYPE_UNKNOWN };

inline string getTypeString(DataType t) {
    switch(t) {
        case TYPE_INT: return "int";
        case TYPE_FLOAT: return "float";
        case TYPE_STRING: return "string";
        case TYPE_BOOL: return "bool";
        case TYPE_VOID: return "void";
        case TYPE_CLASS: return "class";
        default: return "unknown";
    }
}

struct ValueWrapper {
    DataType type;
    int iVal = 0;
    float fVal = 0.0;
    string sVal = "";
    bool bVal = false;
    
    map<string, ValueWrapper> instanceMembers; 

    ValueWrapper() : type(TYPE_UNKNOWN) {}
};

class SymbolTable; 

struct SymbolInfo {
    string name;
    string kind;
    DataType type;
    string className;
    
    vector<DataType> paramTypes;
    vector<string> paramNames; 
    map<string, DataType> classMembers;

    ValueWrapper runtimeValue;
    map<string, ValueWrapper> instanceMembers;

    vector<ASTNode*> funcBody; 
    SymbolTable* funcScopeRef;

    SymbolInfo(string n, string k, DataType t) : name(n), kind(k), type(t), funcScopeRef(NULL) {
        runtimeValue.type = t;
    }
};

class SymbolTable {
public:
    string scopeName;
    SymbolTable* parent;
    map<string, SymbolInfo*> symbols;

    SymbolTable(string name, SymbolTable* p = NULL) : scopeName(name), parent(p) {}

    bool add(SymbolInfo* s) {
        if (symbols.count(s->name)) return false;
        symbols[s->name] = s;
        return true;
    }

    SymbolInfo* lookup(string name) {
        if (symbols.count(name)) return symbols[name];
        if (parent) return parent->lookup(name);
        return NULL;
    }

    void printTable(ofstream& file) {
        file << "Scope: " << scopeName;
        if (parent) file << "  (Parent: " << parent->scopeName << ")";
        file << endl;
        
        for (auto const& entry : symbols) {
            SymbolInfo* val = entry.second;
            file << "Name: " << val->name << " | Kind: " << val->kind << " | Type: " << getTypeString(val->type);
            
            if (val->kind == "function") {
                file << " | Params: (";
                for(auto t : val->paramTypes) file << getTypeString(t) << ", ";
                file << ")";
            }
            if (val->kind == "class") {
                file << " | Fields: " << val->classMembers.size();
            }
            file << endl;
        }
        file << endl;
    }

    ~SymbolTable() {
        for (auto const& entry : symbols) {
            delete entry.second;
        }
    }
};

#endif