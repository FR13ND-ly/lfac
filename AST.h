#ifndef AST_H
#define AST_H

#include "SymbolTable.h"
#include <vector>
#include <iostream>

// Clasa de bază pentru nodurile AST
class ASTNode {
public:
    DataType nodeType;
    virtual ~ASTNode() {}
    virtual ValueWrapper eval() = 0; 
};

// Nod pentru literale (constante)
class LiteralNode : public ASTNode {
    ValueWrapper val;
public:
    LiteralNode(int v) { val.type = TYPE_INT; val.iVal = v; nodeType = TYPE_INT; }
    LiteralNode(float v) { val.type = TYPE_FLOAT; val.fVal = v; nodeType = TYPE_FLOAT; }
    LiteralNode(string v) { val.type = TYPE_STRING; val.sVal = v; nodeType = TYPE_STRING; }
    LiteralNode(bool v) { val.type = TYPE_BOOL; val.bVal = v; nodeType = TYPE_BOOL; }
    
    ValueWrapper eval() override { return val; }
};

// Nod pentru identificatori (variabile simple)
class IdNode : public ASTNode {
    string name;
    SymbolTable* currentScope;
public:
    IdNode(string n, SymbolTable* s) : name(n), currentScope(s) {
        SymbolInfo* sym = s->lookup(name);
        nodeType = sym ? sym->type : TYPE_UNKNOWN;
    }
    ValueWrapper eval() override {
        SymbolInfo* sym = currentScope->lookup(name);
        if (sym) return sym->runtimeValue;
        return ValueWrapper();
    }
};

// Nod pentru acces membri clasă (obj.field)
class MemberAccessNode : public ASTNode {
    string objName;
    string memberName;
    SymbolTable* scope;
public:
    MemberAccessNode(string obj, string mem, SymbolTable* s, DataType t) 
        : objName(obj), memberName(mem), scope(s) { nodeType = t; }
    
    ValueWrapper eval() override {
        // Într-o implementare completă, am căuta instanța specifică în memorie.
        // Pentru această temă, returnăm o valoare default a tipului membrului,
        // deoarece nu avem un heap manager complet implementat.
        ValueWrapper v; 
        v.type = nodeType;
        return v;
    }
};

// Nod pentru apeluri de funcții
class CallNode : public ASTNode {
    string funcName;
    vector<ASTNode*> args;
public:
    CallNode(string name, vector<ASTNode*> a, DataType retType) 
        : funcName(name), args(a) { nodeType = retType; }
    
    ValueWrapper eval() override {
        // Evaluăm argumentele (chiar dacă nu executăm corpul funcției complet aici)
        for(auto arg : args) {
            arg->eval();
        }
        // Returnăm o valoare default pentru tipul returnat
        ValueWrapper v; 
        v.type = nodeType;
        return v;
    }
};

// Nod pentru operații binare (+, *, ==, etc)
class BinaryNode : public ASTNode {
    ASTNode *left, *right;
    string op;
public:
    BinaryNode(ASTNode* l, string o, ASTNode* r) : left(l), op(o), right(r) {
        if (op == "==" || op == "<" || op == ">" || op == "&&" || op == "||") nodeType = TYPE_BOOL;
        else nodeType = l->nodeType; 
    }
    ValueWrapper eval() override {
        ValueWrapper lv = left->eval();
        ValueWrapper rv = right->eval();
        ValueWrapper res; res.type = nodeType;
        
        if (op == "+") {
            if (lv.type == TYPE_INT) res.iVal = lv.iVal + rv.iVal;
            else if (lv.type == TYPE_FLOAT) res.fVal = lv.fVal + rv.fVal;
        } else if (op == "*") {
            if (lv.type == TYPE_INT) res.iVal = lv.iVal * rv.iVal;
        } else if (op == "==") {
            if (lv.type == TYPE_INT) res.bVal = (lv.iVal == rv.iVal);
            else if (lv.type == TYPE_BOOL) res.bVal = (lv.bVal == rv.bVal);
        } else if (op == ">") {
             if (lv.type == TYPE_INT) res.bVal = (lv.iVal > rv.iVal);
             else if (lv.type == TYPE_FLOAT) res.bVal = (lv.fVal > rv.fVal);
        }
        return res;
    }
};

// Nod pentru asignări (ID = expr sau ID.field = expr)
class AssignNode : public ASTNode {
    string idName;
    string memberName; // gol dacă e variabilă simplă
    bool isMemberAccess;
    SymbolTable* scope;
    ASTNode* expression;
public:
    // Constructor variabilă simplă
    AssignNode(string name, SymbolTable* s, ASTNode* e) 
        : idName(name), memberName(""), isMemberAccess(false), scope(s), expression(e) {
        nodeType = e->nodeType;
    }

    ValueWrapper eval() override {
        ValueWrapper val = expression->eval();
        if (!isMemberAccess) {
            SymbolInfo* sym = scope->lookup(idName);
            if (sym) sym->runtimeValue = val; 
        }
        return val;
    }
};

// Nod pentru funcția predefinită Print
class PrintNode : public ASTNode {
    ASTNode* expression;
public:
    PrintNode(ASTNode* e) : expression(e) { nodeType = TYPE_VOID; }
    ValueWrapper eval() override {
        ValueWrapper v = expression->eval();
        std::cout << "[PROGRAM OUTPUT]: ";
        if (v.type == TYPE_INT) std::cout << v.iVal;
        else if (v.type == TYPE_FLOAT) std::cout << v.fVal;
        else if (v.type == TYPE_STRING) std::cout << v.sVal;
        else if (v.type == TYPE_BOOL) std::cout << (v.bVal ? "true" : "false");
        std::cout << std::endl;
        return v;
    }
};

#endif