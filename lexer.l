%{
#include <stdio.h>
#include "parser.tab.h"
#include <string>
#include <iostream>
using namespace std;
%}

%option noyywrap

%%
"int"|"float"|"string"|"bool"|"void" { yylval.Str = new string(yytext); return TYPE; }
"class"     { return CLASS; }
"if"        { return IF; }
"else"      { return ELSE; }
"while"     { return WHILE; }
"main"      { return MAIN; }
"return"    { return RETURN; }
"Print"     { return PRINT; }
"true"|"false" { yylval.Str = new string(yytext); return BOOL_VAL; }
"new"       { return NEW; }

[a-zA-Z][a-zA-Z0-9]* { yylval.Str = new string(yytext); return ID; }
[0-9]+          { yylval.NR = atoi(yytext); return NR; }
[0-9]+\.[0-9]+  { yylval.FLOAT_NR = atof(yytext); return FLOAT_NR; }
\"[^\"]*\"      { yylval.Str = new string(yytext); return STRING_VAL; }

";" { return SEMICOLON; }
"," { return COMMA; }
"." { return DOT; }
"=" { return ASSIGN; }
"+" { return PLUS; }
"-" { return MINUS; }
"*" { return MUL; }
"/" { return DIV; }
"&&" { return AND; }
"||" { return OR; }
"!" { return NOT; }
"==" { return EQ; }
"!=" { return NEQ; }
"<" { return LT; }
">" { return GT; }
"(" { return LPAREN; }
")" { return RPAREN; }
"{" { return LBRACE; }
"}" { return RBRACE; }

[ \t]+ { }
\n { yylineno++; }
. { printf("Unknown character on line %d: %s\n", yylineno, yytext); }

%%