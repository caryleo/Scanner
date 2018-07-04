//
// Created by Gary on 2018.7.4.
//

#include "Constants.h"


//关键词对应的char数组
char ca_Keyword[100][13] = {
        "abstract", "boolean", "break",
        "byte", "case", "catch", "char",
        "class", "const", "continue",
        "default", "do", "double", "else",
        "extends", "false", "final", "finally",
        "float", "for", "goto", "if",
        "implements", "import", "instanceof",
        "int", "interface", "long", "native",
        "new", "null", "package", "private",
        "protected", "public", "return", "short",
        "static", "super", "switch", "synchronized",
        "this", "throw", "throws", "transient",
        "true", "try", "void", "volatile", "while",
        "{", "}", "[", "]", "(", ")", ",", ".", ";",
        "=", "+", "-", "*", "/", "%",
        ">", ">=", "==", "!=", "<", "<=",
        "&&", "||", "!",
        "&", "|", "^", "~",
        "+=", "-=", "*=", "/=", "%=",
        "++", "--",
        "&=", "|=", "^=",
        "<<", ">>", ">>>",
        "<<=", ">>=", ">>>=",
        "?", ":"
};

const int CON_MAX_WORD = 8;
const int CON_MAX_LINE = 256;
const int CON_MAX_FILE = 65536;
const int CON_TABLE_LENGTH = K_TABLE_END;
const int CON_PRIORITY_LENGTH = 14;
const int CON_OPR_TABLE_OFFSET = 50;