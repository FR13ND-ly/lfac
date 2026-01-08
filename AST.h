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

class UnaryNode : public ASTNode {
    ASTNode* expr;
    string op;
public:
    UnaryNode(string o, ASTNode* e) : op(o), expr(e) {
        nodeType = e->nodeType;
    }
    ~UnaryNode() { delete expr; }

    ValueWrapper eval() override {
        ValueWrapper v = expr->eval();
        if (op == "!" && v.type == TYPE_BOOL) {
            v.bVal = !v.bVal;
        }
        return v;
    }
};

class BinaryNode : public ASTNode {
    ASTNode *left, *right;
    string op; 
public:
    BinaryNode(ASTNode* l, string o, ASTNode* r) : left(l), op(o), right(r) {
        if (op == ">" || op == "<" || op == "==" || op == "&&" || op == "||") {
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
        else if (l.type == TYPE_BOOL) {
            if (op == "&&") res.bVal = l.bVal && r.bVal;
            else if (op == "||") res.bVal = l.bVal || r.bVal;
            else if (op == "==") res.bVal = (l.bVal == r.bVal);
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
    vector<ASTNode*> thenBody;
    vector<ASTNode*> elseBody;
public:
    IfNode(ASTNode* c, vector<ASTNode*> t, vector<ASTNode*> e = {}) 
        : cond(c), thenBody(t), elseBody(e) { nodeType = TYPE_VOID; }
    
    ~IfNode() { 
        delete cond; 
        for(auto n : thenBody) delete n; 
        for(auto n : elseBody) delete n; 
    }
    
    ValueWrapper eval() override {
        ValueWrapper res = cond->eval();
        if (res.bVal) {
            for(auto n : thenBody) if(n) n->eval();
        } else {
            for(auto n : elseBody) if(n) n->eval();
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

class ReturnNode : public ASTNode {
    ASTNode* expr;
public:
    ReturnNode(ASTNode* e) : expr(e) { 
        nodeType = e->nodeType; 
    }
    ~ReturnNode() { delete expr; }

    ValueWrapper eval() override {
        ValueWrapper v = expr->eval();
        throw v; 
    }
};

class FunctionCallNode : public ASTNode {
    string funcName;
    vector<ASTNode*> args;
    SymbolTable* currentScope;
    DataType retType;
public:
    FunctionCallNode(string name, vector<ASTNode*> a, SymbolTable* s, DataType rt) 
        : funcName(name), args(a), currentScope(s), retType(rt) {
        nodeType = retType;
    }

    ValueWrapper eval() override {
        SymbolTable* global = currentScope;
        while(global->parent != NULL) global = global->parent;
        SymbolInfo* funcSym = global->lookup(funcName);

        if (!funcSym) return ValueWrapper();

        vector<ValueWrapper> argVals;
        for(auto arg : args) argVals.push_back(arg->eval());

        SymbolTable* targetScope = funcSym->funcScopeRef;
        if(targetScope) {
            for(size_t i=0; i<funcSym->paramNames.size(); i++) {
                string pName = funcSym->paramNames[i];
                SymbolInfo* paramSym = targetScope->lookup(pName);
                if(paramSym) {
                    paramSym->runtimeValue = argVals[i];
                }
            }
        }

        try {
            for(ASTNode* node : funcSym->funcBody) {
                if(node) node->eval();
            }
        } catch (ValueWrapper retVal) {
            return retVal;
        }

        return ValueWrapper();
    }
};

class RealMethodCallNode : public ASTNode {
    string objName;
    vector<ASTNode*> args;
    SymbolInfo* methodInfo; 
    SymbolTable* currentScope;
public:
    RealMethodCallNode(string on, vector<ASTNode*> a, SymbolInfo* minfo, SymbolTable* scope)
        : objName(on), args(a), methodInfo(minfo), currentScope(scope) {
        nodeType = minfo->type;
    }

    ValueWrapper eval() override {
        vector<ValueWrapper> argVals;
        for(auto arg : args) argVals.push_back(arg->eval());

        SymbolTable* targetScope = methodInfo->funcScopeRef;
        if(targetScope) {
            for(size_t i=0; i<methodInfo->paramNames.size(); i++) {
                string pName = methodInfo->paramNames[i];
                SymbolInfo* paramSym = targetScope->lookup(pName);
                if(paramSym) {
                    paramSym->runtimeValue = argVals[i];
                }
            }
        }

        try {
            for(ASTNode* node : methodInfo->funcBody) {
                if(node) node->eval();
            }
        } catch (ValueWrapper retVal) {
            return retVal;
        }
        return ValueWrapper();
    }
};

#endif