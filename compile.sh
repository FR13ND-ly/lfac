#!/bin/bash

set -e

rm -f lex.yy.c parser.tab.c parser.tab.h compiler tables.txt

bison -d parser.y

flex lexer.l

g++ -std=c++17 lex.yy.c parser.tab.c -o compiler

./compiler inputOk.txt
