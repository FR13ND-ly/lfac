#!/bin/bash

# Oprește scriptul imediat dacă o comandă eșuează
set -e

# Curățare fișiere vechi
rm -f lex.yy.c parser.tab.c parser.tab.h compiler tables.txt

echo "Pasul 1: Bison..."
bison -d parser.y

echo "Pasul 2: Flex..."
flex lexer.l

echo "Pasul 3: G++ Compilation..."
g++ -std=c++17 lex.yy.c parser.tab.c -o compiler

echo "Pasul 4: Execuție..."
if [ -f "test.txt" ]; then
    ./compiler test.txt
else
    echo "Creează test.txt pentru a rula compilatorul."
fi