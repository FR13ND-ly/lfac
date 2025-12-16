%{
#include <stdio.h>
#include "LFAC.tab.h"
#include <string>
%}
%option noyywrap

%%
"int"|"float"|"char"|"string"|"bool" {yylval.Str = new std::string(yytext); return TYPE;}
[a-zA-Z][a-zA-Z0-9]* {yylval.Str = new std::string(yytext); return ID;}
";" { return SEMICOLON;}
[0-9]+ { yylval.NR = atoi(yytext); return NR; }
[0-9]+\.[0-9]+ { yylval.Str = new std::string(yytext); return FLOAT_NR; }
"=" {return ASSIGN;}
[ \t]+ { }
\n {yylineno++;}
. {printf("caracter necunoscut:%s\n",yytext);}

%%
