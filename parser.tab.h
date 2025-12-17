/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_PARSER_TAB_H_INCLUDED
# define YY_YY_PARSER_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif
/* "%code requires" blocks.  */
#line 27 "parser.y"

  #include <string>
  using namespace std;

#line 54 "parser.tab.h"

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    ASSIGN = 258,                  /* ASSIGN  */
    SEMICOLON = 259,               /* SEMICOLON  */
    COMMA = 260,                   /* COMMA  */
    DOT = 261,                     /* DOT  */
    PLUS = 262,                    /* PLUS  */
    MINUS = 263,                   /* MINUS  */
    MUL = 264,                     /* MUL  */
    DIV = 265,                     /* DIV  */
    AND = 266,                     /* AND  */
    OR = 267,                      /* OR  */
    NOT = 268,                     /* NOT  */
    EQ = 269,                      /* EQ  */
    NEQ = 270,                     /* NEQ  */
    LT = 271,                      /* LT  */
    GT = 272,                      /* GT  */
    LPAREN = 273,                  /* LPAREN  */
    RPAREN = 274,                  /* RPAREN  */
    LBRACE = 275,                  /* LBRACE  */
    RBRACE = 276,                  /* RBRACE  */
    CLASS = 277,                   /* CLASS  */
    IF = 278,                      /* IF  */
    ELSE = 279,                    /* ELSE  */
    WHILE = 280,                   /* WHILE  */
    MAIN = 281,                    /* MAIN  */
    RETURN = 282,                  /* RETURN  */
    PRINT = 283,                   /* PRINT  */
    NEW = 284,                     /* NEW  */
    NR = 285,                      /* NR  */
    FLOAT_NR = 286,                /* FLOAT_NR  */
    ID = 287,                      /* ID  */
    TYPE = 288,                    /* TYPE  */
    BOOL_VAL = 289,                /* BOOL_VAL  */
    STRING_VAL = 290,              /* STRING_VAL  */
    LOWER_THAN_ELSE = 291          /* LOWER_THAN_ELSE  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 32 "parser.y"

    std::string* Str;
    int NR;
    float FLOAT_NR;

#line 113 "parser.tab.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);


#endif /* !YY_YY_PARSER_TAB_H_INCLUDED  */
