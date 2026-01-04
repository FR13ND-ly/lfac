%{
    #include "AST.h"
    #include "parser.tab.h"
    #include <string>
    using namespace std;
%}

%option noyywrap
%option yylineno

%%

"let"           { return LET; }
"class"         { return CLASS; }
"function"      { return FUNCTION; }
"main"          { return MAIN; }
"if"            { return IF; }
"else"          { return ELSE; }
"while"         { return WHILE; }
"return"        { return RETURN; }
"Print"         { return PRINT; }

"int"           { yylval.dataType = TYPE_INT; return T_INT; }
"float"         { yylval.dataType = TYPE_FLOAT; return T_FLOAT; }
"string"        { yylval.dataType = TYPE_STRING; return T_STRING; }
"bool"          { yylval.dataType = TYPE_BOOL; return T_BOOL; }
"void"          { yylval.dataType = TYPE_VOID; return T_VOID; }

"true"          { yylval.boolVal = true; return BOOL_VAL; }
"false"         { yylval.boolVal = false; return BOOL_VAL; }

[0-9]+\.[0-9]+  { yylval.floatVal = stof(yytext); return FLOAT_VAL; }
[0-9]+          { yylval.intVal = atoi(yytext); return INT_VAL; }

\"[^\"]*\"      { 
    string s = yytext; 
    yylval.strVal = new string(s.substr(1, s.length()-2)); 
    return STRING_VAL; 
}

[a-zA-Z_][a-zA-Z0-9_]* { 
    yylval.strVal = new string(yytext); 
    return ID; 
}

":"             { return COLON; }
"=="            { return EQ; }
"&&"            { return AND; }
"||"            { return OR; }
"."             { return '.'; }

[ \t\r\n]       ; /* Ignorăm whitespace */
.               { return yytext[0]; }

%%