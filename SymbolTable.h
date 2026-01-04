#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <fstream>

using namespace std;

// Tipuri de date acceptate
enum DataType { TYPE_INT, TYPE_FLOAT, TYPE_STRING, TYPE_BOOL, TYPE_VOID, TYPE_CLASS, TYPE_UNKNOWN };

// Wrapper pentru valorile de la runtime
struct ValueWrapper {
    DataType type;
    int iVal = 0;
    float fVal = 0.0;
    string sVal = "";
    bool bVal = false;
    ValueWrapper() : type(TYPE_UNKNOWN) {}
};

// Informații despre un simbol
struct SymbolInfo {
    string name;
    string kind; // "variable", "function", "class", "parameter", "field"
    DataType type;
    string className; // Numele clasei dacă simbolul este un obiect
    ValueWrapper runtimeValue;
    
    // Pentru funcții: semnătura parametrilor
    vector<DataType> parameterTypes;
    
    // Pentru clase: membrii (fields) cu tipul lor
    map<string, DataType> members;

    SymbolInfo(string n, string k, DataType t) : name(n), kind(k), type(t) {
        runtimeValue.type = t;
    }
};

// Clasa pentru Tabela de Simboluri
class SymbolTable {
public:
    string scopeName;
    SymbolTable* parent;
    map<string, SymbolInfo*> symbols;

    SymbolTable(string name, SymbolTable* p = nullptr) : scopeName(name), parent(p) {}

    ~SymbolTable() {
        for (auto const& [key, val] : symbols) delete val;
    }

    bool add(SymbolInfo* s) {
        if (symbols.count(s->name)) return false;
        symbols[s->name] = s;
        return true;
    }

    SymbolInfo* lookup(string name) {
        if (symbols.count(name)) return symbols[name];
        if (parent) return parent->lookup(name);
        return nullptr;
    }

    void printTable(ofstream& file, int level = 0) {
        string indent(level * 4, ' ');
        file << indent << "--- SCOPE: " << scopeName << " ---\n";
        for (auto const& [name, sym] : symbols) {
            file << indent << "  [" << sym->kind << "] " << name << " : type " << sym->type;
            if (sym->kind == "function") {
                file << " (Params: " << sym->parameterTypes.size() << ")";
            }
            if (sym->kind == "class") {
                file << " (Members: " << sym->members.size() << ")";
            }
            file << "\n";
        }
        file << "\n";
    }
};

#endif