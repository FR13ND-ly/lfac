#include <iostream>
#include <map>
#include <string>

using namespace std;

class IdInfo {
    public:
    string type;
    string name;
    int intValue;
    float floatValue;
    bool boolValue;
    string strValue;
   
    IdInfo() : intValue(0), floatValue(0.0), boolValue(false), strValue("") {}
    IdInfo(string* type, string* name) : type(*type), name(*name) {}
    
};

class SymTable {
    SymTable* parent;
    map<string, IdInfo> ids;
    string name;
    public:
    SymTable(const char* tableName, SymTable* parent = NULL) 
    : name(tableName), parent(parent) {}
    bool existsId(string* s);
    void addVar(string* type, string* name );
    void setValue(string* name, int value);
    void setValue(string* name, float value);
    void setValue(string* name, bool value);
    void setValue(string* name, string value);


    void printVars();
    ~SymTable();
};






