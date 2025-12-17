CC = g++
CFLAGS = -std=c++11 -Wall

TARGET = compiler

BISON_FILE = parser.y
FLEX_FILE = lexer.l
CPP_FILES = main.cpp SymTable.cpp

BISON_GEN_C = parser.tab.c
BISON_GEN_H = parser.tab.h
FLEX_GEN = lex.yy.c

all: $(TARGET)

# Link everything together
$(TARGET): $(BISON_GEN_C) $(FLEX_GEN) $(CPP_FILES)
	$(CC) $(CFLAGS) $(BISON_GEN_C) $(FLEX_GEN) $(CPP_FILES) -o $(TARGET)

# Run Bison to generate parser code
$(BISON_GEN_C) $(BISON_GEN_H): $(BISON_FILE)
	bison -d $(BISON_FILE)

# Run Flex to generate lexer code
$(FLEX_GEN): $(FLEX_FILE)
	flex $(FLEX_FILE)

# Clean build files
clean:
	rm -f $(TARGET) $(BISON_GEN_C) $(BISON_GEN_H) $(FLEX_GEN)

# Helper to run the test
run: $(TARGET)
	./$(TARGET) test.txt