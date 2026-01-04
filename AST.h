#ifndef AST_H
#define AST_H

#include "SymbolTable.h"
#include <iostream>
#include <string>
#include <vector>

using namespace std;

class ASTNode {
public:
    DataType nodeType;
    virtual ~ASTNode() {}
    virtual ValueWrapper eval() = 0; 
};

class LiteralNode : public ASTNode {
    ValueWrapper val;
public:
    LiteralNode(int v) { val.type = TYPE_INT; val.iVal = v; nodeType = TYPE_INT; }
    LiteralNode(float v) { val.type = TYPE_FLOAT; val.fVal = v; nodeType = TYPE_FLOAT; }
    LiteralNode(string v) { val.type = TYPE_STRING; val.sVal = v; nodeType = TYPE_STRING; }
    LiteralNode(bool v) { val.type = TYPE_BOOL; val.bVal = v; nodeType = TYPE_BOOL; }
    ValueWrapper eval() override { return val; }
};

class IdNode : public ASTNode {
    string name;
    SymbolTable* scope;
public:
    IdNode(string n, SymbolTable* s) : name(n), scope(s) {
        SymbolInfo* sym = s->lookup(n);
        if(sym) nodeType = sym->type;
        else nodeType = TYPE_UNKNOWN;
    }
    ValueWrapper eval() override {
        SymbolInfo* sym = scope->lookup(name);
        if (sym) return sym->runtimeValue;
        return ValueWrapper();
    }
};

class MemberAccessNode : public ASTNode {
    string objName;
    string fieldName;
    SymbolTable* scope;
public:
    MemberAccessNode(string obj, string field, SymbolTable* s, DataType realType) 
        : objName(obj), fieldName(field), scope(s) {
        nodeType = realType; 
    }

    ValueWrapper eval() override {
        SymbolInfo* obj = scope->lookup(objName);
        if (obj && obj->instanceMembers.count(fieldName)) {
            return obj->instanceMembers[fieldName];
        }
        return ValueWrapper();
    }
};

class BinaryNode : public ASTNode {
    ASTNode *left, *right;
    string op; 
public:
    BinaryNode(ASTNode* l, string o, ASTNode* r) : left(l), op(o), right(r) {
        if (op == ">" || op == "<" || op == "==") {
            nodeType = TYPE_BOOL;
        } else {
            nodeType = l->nodeType; 
        }
    }
    ~BinaryNode() { delete left; delete right; }

    ValueWrapper eval() override {
        ValueWrapper l = left->eval();
        ValueWrapper r = right->eval();
        ValueWrapper res;
        
        if (l.type != r.type) return res;

        res.type = nodeType;

        if (l.type == TYPE_INT) {
            if (op == "+") res.iVal = l.iVal + r.iVal;
            else if (op == "-") res.iVal = l.iVal - r.iVal;
            else if (op == "*") res.iVal = l.iVal * r.iVal;
            else if (op == "/") res.iVal = l.iVal / r.iVal;
            else if (op == ">") res.bVal = l.iVal > r.iVal;
            else if (op == "<") res.bVal = l.iVal < r.iVal;
            else if (op == "==") res.bVal = l.iVal == r.iVal;
        }
        else if (l.type == TYPE_FLOAT) {
             if (op == "+") res.fVal = l.fVal + r.fVal;
             else if (op == "-") res.fVal = l.fVal - r.fVal;
             else if (op == "*") res.fVal = l.fVal * r.fVal;
             else if (op == "/") res.fVal = l.fVal / r.fVal;
             else if (op == ">") res.bVal = l.fVal > r.fVal;
             else if (op == "<") res.bVal = l.fVal < r.fVal;
             else if (op == "==") res.bVal = l.fVal == r.fVal;
        }
        else if (l.type == TYPE_STRING && op == "+") {
            res.sVal = l.sVal + r.sVal;
        }
        return res;
    }
};

class NewNode : public ASTNode {
    string className;
    SymbolTable* globalScope;
public:
    NewNode(string cls, SymbolTable* gs) : className(cls), globalScope(gs) {
        nodeType = TYPE_CLASS;
    }
    ValueWrapper eval() override {
        ValueWrapper v;
        v.type = TYPE_CLASS;
        v.sVal = className; 
        
        SymbolInfo* clsDef = globalScope->lookup(className);
        if(clsDef){
            for(auto const& entry : clsDef->classMembers){
                ValueWrapper defVal; 
                defVal.type = entry.second;
                v.instanceMembers[entry.first] = defVal;
            }
        }
        return v;
    }
};

class AssignNode : public ASTNode {
    string name;
    string memberName; 
    bool isMemberAccess;
    SymbolTable* scope;
    ASTNode* expr;
public:
    AssignNode(string n, SymbolTable* s, ASTNode* e) 
        : name(n), memberName(""), isMemberAccess(false), scope(s), expr(e) {
        nodeType = e->nodeType;
    }

    AssignNode(string n, string m, SymbolTable* s, ASTNode* e)
        : name(n), memberName(m), isMemberAccess(true), scope(s), expr(e) {
        nodeType = e->nodeType;
    }

    ~AssignNode() { delete expr; }

    ValueWrapper eval() override {
        ValueWrapper val = expr->eval();
        SymbolInfo* sym = scope->lookup(name);

        if (sym) {
            if (!isMemberAccess) {
                sym->runtimeValue = val;
                if (val.type == TYPE_CLASS) {
                    sym->className = val.sVal;
                    sym->instanceMembers = val.instanceMembers;
                }
            } else {
                sym->instanceMembers[memberName] = val;
            }
        }
        return val;
    }
};

class PrintNode : public ASTNode {
    ASTNode* expr;
public:
    PrintNode(ASTNode* e) : expr(e) { nodeType = TYPE_VOID; }
    ~PrintNode() { delete expr; }
    
    ValueWrapper eval() override {
        ValueWrapper v = expr->eval();
        if (v.type == TYPE_INT) cout << "[PRINT]: " << v.iVal << endl;
        else if (v.type == TYPE_FLOAT) cout << "[PRINT]: " << v.fVal << endl;
        else if (v.type == TYPE_STRING) cout << "[PRINT]: " << v.sVal << endl;
        else if (v.type == TYPE_BOOL) cout << "[PRINT]: " << (v.bVal ? "true" : "false") << endl;
        else if (v.type == TYPE_CLASS) cout << "[PRINT]: Object<" << v.sVal << ">" << endl;
        return v;
    }
};

class IfNode : public ASTNode {
    ASTNode* cond;
    vector<ASTNode*> body;
public:
    IfNode(ASTNode* c, vector<ASTNode*> b) : cond(c), body(b) { nodeType = TYPE_VOID; }
    ~IfNode() { delete cond; for(auto n : body) delete n; }
    
    ValueWrapper eval() override {
        ValueWrapper res = cond->eval();
        if (res.bVal) {
            for(auto n : body) n->eval();
        }
        return ValueWrapper();
    }
};

class WhileNode : public ASTNode {
    ASTNode* cond;
    vector<ASTNode*> body;
public:
    WhileNode(ASTNode* c, vector<ASTNode*> b) : cond(c), body(b) { nodeType = TYPE_VOID; }
    ~WhileNode() { delete cond; for(auto n : body) delete n; }

    ValueWrapper eval() override {
        while(true) {
            ValueWrapper res = cond->eval();
            if(!res.bVal) break;
            for(auto n : body) n->eval();
        }
        return ValueWrapper();
    }
};

#endif