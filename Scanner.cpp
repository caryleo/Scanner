//
// Created by Gary on 2018.7.3.
//

#include <cstring>
#include "Scanner.h"

Scanner::~Scanner() = default;

Scanner::Scanner() = default;

void Scanner::Execute() {
    if_InFile.open(str_InFilePathName, ios::in);
    if (if_InFile.good()) {
        cout << "正在分析，请稍后……" << endl;
        of_OutFile.open(str_OutFileName, ios::out);
        of_ErrFile.open(str_ErrFileName, ios::out);
        //读满一个缓冲区
        _LoadToLo();
        _LoToHi();
        _LoadToLo();
        //开始进行扫描
        while (true) {
            _Scan();
            if (b_TrueEndTag) {
                break;
            }
        }
        _FileOutput();
        _ErrOutput();
        of_OutFile.close();
        of_ErrFile.close();
        cout << "分析完成" << endl;
    }
    else {
        cerr << "文件读取错误！（文件错误或无此文件）" << endl;
    }
    if_InFile.close();
}

void Scanner::Init(string fileName) {
    //对文件名进行加工
    str_InFileName = fileName;
    str_InFilePathName = CON_FILE_PATH_PREFIX + str_InFileName + ".java";
    str_OutFileName = CON_FILE_PATH_PREFIX + str_InFileName + "_Out.txt";
    str_ErrFileName = CON_FILE_PATH_PREFIX + str_InFileName + "_Err.txt";
    str_Buffer.clear();
    str_BufferL.clear();
    str_BufferR.clear();
    for (int i = 0; i < 64; ++i) {
        str_Buffer.append("#");
        str_BufferL.append("#");
        str_BufferR.append("#");
    }
    i_ForwardCount = 0;
    b_IsRightBufferAvail = false;
    b_EOFTag = false;
    b_EndTag = false;
    b_TrueEndTag = false;
    //默认初始化下，行号为1，列号为0
    i_Line = 1;
    i_Col = 0;
    v_Tokens.clear();
}

char Scanner::_ReadChar() {
    char ret;
    //cout << "读入一个字符" << endl;
    if (i_ForwardCount >= (CON_BUFFER_SIZE / 2)) {
        //读完一个缓冲区，需要加载下一个缓冲区
        if (b_IsRightBufferAvail) {
            //第二个缓冲区里面有东西
            _LoToHi();
            _LoToR();
        }
        else if (!b_EOFTag) {
            _LoToHi();
            _LoadToLo();
        }
        else if (!b_EndTag) {
            _LoToHi();
            b_EndTag = true;
        }
        else {
            return EOF;
        }
        i_ForwardCount = 0;
    }
    ret = str_Buffer.at(i_ForwardCount++);
    return ret;
}

void Scanner::_LoadToLo() {
    char str[CON_BUFFER_SIZE / 2 + 1];
    memset(str, 0, sizeof(str));
    if_InFile.read(str, CON_BUFFER_SIZE / 2);
    if (if_InFile.eof()) {
        b_EOFTag = true;
    }
    for (int i = CON_BUFFER_SIZE / 2; i < CON_BUFFER_SIZE; ++i) {
        str_Buffer.at(i) = str[i - CON_BUFFER_SIZE / 2];
    }
}

void Scanner::_LoToHi() {
    for (int i = 0; i < CON_BUFFER_SIZE / 2; ++i) {
        str_BufferL.at(i) = str_Buffer.at(i);
        str_Buffer.at(i) = str_Buffer.at(i + CON_BUFFER_SIZE / 2);
    }
}

void Scanner::_LoToR() {
    for (int i = 0; i < CON_BUFFER_SIZE / 2; ++i) {
        str_Buffer.at(i + CON_BUFFER_SIZE / 2) = str_BufferR.at(i);
    }
    b_IsRightBufferAvail = false;
}

void Scanner::_Untread() {
    i_ForwardCount--;
    if (i_ForwardCount < 0) {
        for (int i = 0; i < CON_BUFFER_SIZE / 2; ++i) {
            str_BufferR.at(i) = str_Buffer.at(i + CON_BUFFER_SIZE / 2);
            str_Buffer.at(i + CON_BUFFER_SIZE / 2) = str_Buffer.at(i);
            str_Buffer.at(i) = str_BufferL.at(i);
        }
        b_IsRightBufferAvail = true;
        i_ForwardCount = CON_BUFFER_SIZE / 2 - 1;
    }

}

void Scanner::_Scan() {
    char ch = _ReadChar();
    //分析结束判断
    if (ch == EOF || (ch == 0 && b_EndTag)) {
        b_TrueEndTag = true;
        return;
    }
    c_CurChar = ch;
    switch (c_CurChar) {
        case '$':
        case '_':
        case 'A':
        case 'a':
        case 'B':
        case 'b':
        case 'C':
        case 'c':
        case 'D':
        case 'd':
        case 'E':
        case 'e':
        case 'F':
        case 'f':
        case 'G':
        case 'g':
        case 'H':
        case 'h':
        case 'I':
        case 'i':
        case 'J':
        case 'j':
        case 'K':
        case 'k':
        case 'L':
        case 'l':
        case 'M':
        case 'm':
        case 'N':
        case 'n':
        case 'O':
        case 'o':
        case 'P':
        case 'p':
        case 'Q':
        case 'q':
        case 'R':
        case 'r':
        case 'S':
        case 's':
        case 'T':
        case 't':
        case 'U':
        case 'u':
        case 'V':
        case 'v':
        case 'W':
        case 'w':
        case 'X':
        case 'x':
        case 'Y':
        case 'y':
        case 'Z':
        case 'z':
            //$，-，和字母开头：标识符、关键字、布尔常量
            i_Col++;//正常字符，列号加一定位
            _ScanIdentityOrKeywordOrBoolean();
            break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            //0和其他数字：整型常量或实型常量
            i_Col++;
            _ScanNumber();
            break;
        case '\n':
            //换行不影响词素识别（对于标识符等，会在内部解决这个问题）
            i_Line++;
            i_Col = 0;
            break;
        case '\t':
            //tab也不会影响词素识别，但是会影响缩进，因此需要重新定位
            i_Col += 4;
            break;
        case ' ':
            //空格
            i_Col++;
            _ScanSpace();
            break;
        case '\'':
            i_Col++;
            _ScanCharacter();
            break;
        case '"':
            i_Col++;
            _ScanString();
            break;
        case '+':
        case '-':
        case '*':
        case '/':
        case '=':
        case '&':
        case '|':
        case '^':
        case '~':
        case '<':
        case '>':
        case '%':
        case '!':
        case '?':
        case ':':
            i_Col++;
            _ScanOPR();
            break;
        case '{':
        case '}':
        case '[':
        case ']':
        case '(':
        case ')':
        case ',':
        case '.':
        case ';':
            i_Col++;
            _ScanBoundary();
            break;
        case '\f':break;
        case '\b':
            //回退一个字符
            _Untread();
            break;
        case '\r':
            //回退到行首
            _Untread();
            while (i_Col--) {
                _Untread();
            }
            break;
        default:
            break;
    }
}

void Scanner::_ScanIdentityOrKeywordOrBoolean() {
    char ch = c_CurChar;
    Token tok;
    Position stPos(i_Line, i_Col);
    string tmpStr;//用于临时存放单词
    tmpStr.clear();
    tmpStr = ch;
    ch = _ReadChar();//继续往后读
    if (ch == EOF || (ch == 0 && b_EndTag)) {
        b_TrueEndTag = true;
        Position edPos(i_Line, i_Col);
        tok.Set(A_LEX_ERROR, K_IDENTITY, tmpStr, stPos, edPos);
        v_Tokens.push_back(tok);
        _LexError(103, tok);
        return;
    }
    while ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || (ch == '$') || (ch == '_') ||
           (ch >= '0' && ch <= '9')) {
        tmpStr += ch;
        i_Col++;
        ch = _ReadChar();
        if (ch == EOF || (ch == 0 && b_EndTag)) {
            b_TrueEndTag = true;
            Position edPos(i_Line, i_Col);
            tok.Set(A_LEX_ERROR, K_IDENTITY, tmpStr, stPos, edPos);
            v_Tokens.push_back(tok);
            _LexError(103, tok);
            return;
        }
    }
    //最后一个字符不符合要求，回退之
    _Untread();
    Position edPos(i_Line, i_Col);
    int tmpB = _IsBoolean(tmpStr);
    int tmpK = _IsKeyword(tmpStr);
    //布尔常量
    if (tmpB != 0) {
        if (tmpB == 1) {
            tok.Set(A_LEX_BOOL, K_CONSTANT_BOOL, "true", stPos, edPos);
        }
        else {
            tok.Set(A_LEX_BOOL, K_CONSTANT_BOOL, "false", stPos, edPos);
        }
        v_Tokens.push_back(tok);
    }
        //关键字
    else if (tmpK != -1) {
        tok.Set(A_LEX_KEYWORD, e_KeywordKind(tmpK), ca_Keyword[tmpK], stPos, edPos);
        v_Tokens.push_back(tok);
    }
        //标识符
    else {
        tok.Set(A_LEX_IDENTITY, K_IDENTITY, tmpStr, stPos, edPos);
        v_Tokens.push_back(tok);
    }

}

int Scanner::_IsBoolean(string str) {
    if (str == "true") {
        return 1;
    }
    else if (str == "false") {
        return 2;
    }
    return 0;
}

int Scanner::_IsKeyword(string str) {
    //50个关键字直接查表
    for (int i = 0; i < 50; ++i) {
        string tmp(ca_Keyword[i]);
        if (str == tmp) {
            return i;
        }
    }
    return -1;
}

void Scanner::_FileOutput() {
    int len = v_Tokens.size();
    for (int i = 0; i < len; ++i) {
        Token tmp = v_Tokens.at(i);
        //起点行列号
        cout << tmp.getPos_StartPos().getI_Line() << '\t' << tmp.getPos_StartPos().getI_Col()
             << '\t';
        of_OutFile << tmp.getPos_StartPos().getI_Line() << '\t' << tmp.getPos_StartPos().getI_Col()
                   << '\t';
        cout << "<" << tmp.getPos_EndPos().getI_Line() << "," << tmp.getPos_EndPos().getI_Col()<< ">"
             << "\t";
        of_OutFile << tmp.getPos_EndPos().getI_Line() << '\t' << tmp.getPos_EndPos().getI_Col()
                   << '\t';
        //type
        e_Attributes type = tmp.getEa_Type();
        of_OutFile << type << '\t';
        switch (type) {
            case A_LEX_ERROR:
                cout << hex << A_LEX_ERROR << "\t";
                cout << dec << "[error]" << "\t";
                break;
            case A_LEX_COMMENT:
                cout << hex << A_LEX_COMMENT << "\t";
                cout << dec << "[comment]" << "\t";
                break;
            case A_LEX_SPACE:
                cout << hex << A_LEX_SPACE << "\t";
                cout << dec << "[space]" << "\t";
                break;
            case A_LEX_KEYWORD:
                cout << hex << A_LEX_KEYWORD << "\t";
                cout << dec << "[keyword]" << "\t";
                break;
            case A_LEX_IDENTITY:
                cout << hex << A_LEX_IDENTITY << "\t";
                cout << dec << "[identity]" << "\t";
                break;
            case A_LEX_BOOL:
                cout << hex << A_LEX_BOOL << "\t";
                cout << dec << "[bool]" << "\t";
                break;
            case A_LEX_CHAR:
                cout << hex << A_LEX_CHAR << "\t";
                cout << dec << "[char]" << "\t";
                break;
            case A_LEX_INT:
                cout << hex << A_LEX_INT << "\t";
                cout << dec << "[int]" << "\t";
                break;
            case A_LEX_REAL:
                cout << hex << A_LEX_REAL << "\t";
                cout << dec << "[real]" << "\t";
                break;
            case A_LEX_STRING:
                cout << hex << A_LEX_STRING << "\t";
                cout << dec << "[string]" << "\t";
                break;
            case A_LEX_ASSIGN:
                cout << hex << A_LEX_ASSIGN << "\t";
                cout << dec << "[assign]" << "\t";
                break;
            case A_LEX_TERNARY:
                cout << hex << A_LEX_TERNARY << "\t";
                cout << dec << "[ternary]" << "\t";
                break;
            case A_LEX_OR:
                cout << hex << A_LEX_OR << "\t";
                cout << dec << "[or]" << "\t";
                break;
            case A_LEX_AND:
                cout << hex << A_LEX_AND << "\t";
                cout << dec << "[and]" << "\t";
                break;
            case A_LEX_OR_BIT:
                cout << hex << A_LEX_OR_BIT << "\t";
                cout << dec << "[or bit]" << "\t";
                break;
            case A_LEX_XOR_BIT:
                cout << hex << A_LEX_XOR_BIT << "\t";
                cout << dec << "[xor bit]" << "\t";
                break;
            case A_LEX_AND_BIT:
                cout << hex << A_LEX_AND_BIT << "\t";
                cout << dec << "[and bit]" << "\t";
                break;
            case A_LEX_EQUAL:
                cout << hex << A_LEX_EQUAL << "\t";
                cout << dec << "[equal]" << "\t";
                break;
            case A_LEX_COMPARE:
                cout << hex << A_LEX_COMPARE << "\t";
                cout << dec << "[compare]" << "\t";
                break;
            case A_LEX_SHIFT:
                cout << hex << A_LEX_SHIFT << "\t";
                cout << dec << "[shift]" << "\t";
                break;
            case A_LEX_ADD_SUB:
                cout << hex << A_LEX_ADD_SUB << "\t";
                cout << dec << "[add sub]" << "\t";
                break;
            case A_LEX_MUL_DIV_MOD:
                cout << hex << A_LEX_MUL_DIV_MOD << "\t";
                cout << dec << "[mul div mod]" << "\t";
                break;
            case A_LEX_HIGH_PRIORITY:
                cout << hex << A_LEX_HIGH_PRIORITY << "\t";
                cout << dec << "[high priority]" << "\t";
                break;
            case A_LEX_BOUNDARY:
                cout << hex << A_LEX_BOUNDARY << "\t";
                cout << dec << "[boundary]" << "\t";
                break;
            case A_LEX_COMMA:
                cout << hex << A_LEX_COMMA << "\t";
                cout << dec << "[comma]" << "\t";
                break;
            case A_LEX_BRACES:
                cout << hex << A_LEX_BRACES << "\t";
                cout << dec << "[braces]" << "\t";
                break;
            case A_LEX_SEMICOLON:
                cout << hex << A_LEX_SEMICOLON << "\t";
                cout << dec << "[semicolon]" << "\t";
                break;
            default:break;
        }
        //kind
        e_KeywordKind kind = tmp.getEk_kind();
        of_OutFile << kind << '\t';
        switch (kind) {
            case K_KEYWORD_ABSTRACT:
                cout << K_KEYWORD_ABSTRACT << "\t";
                cout << "{keyword abstract}" << "\t";
                break;
            case K_KEYWORD_BOOLEAN:
                cout << K_KEYWORD_BOOLEAN << "\t";
                cout << "{keyword boolean}" << "\t";
                break;
            case K_KEYWORD_BREAK:
                cout << K_KEYWORD_BREAK << "\t";
                cout << "{keyword break}" << "\t";
                break;
            case K_KEYWORD_BYTE:
                cout << K_KEYWORD_BYTE << "\t";
                cout << "{keyword byte}" << "\t";
                break;
            case K_KEYWORD_CASE:
                cout << K_KEYWORD_CASE << "\t";
                cout << "{keyword case}" << "\t";
                break;
            case K_KEYWORD_CATCH:
                cout << K_KEYWORD_CATCH << "\t";
                cout << "{keyword catch}" << "\t";
                break;
            case K_KEYWORD_CHAR:
                cout << K_KEYWORD_CHAR << "\t";
                cout << "{keyword char}" << "\t";
                break;
            case K_KEYWORD_CLASS:
                cout << K_KEYWORD_CLASS << "\t";
                cout << "{keyword class}" << "\t";
                break;
            case K_KEYWORD_CONST:
                cout << K_KEYWORD_CONST << "\t";
                cout << "{keyword const}" << "\t";
                break;
            case K_KEYWORD_CONTINUE:
                cout << K_KEYWORD_CONTINUE << "\t";
                cout << "{keyword continue}" << "\t";
                break;
            case K_KEYWORD_DEFAULT:
                cout << K_KEYWORD_DEFAULT << "\t";
                cout << "{keyword default}" << "\t";
                break;
            case K_KEYWORD_DO:
                cout << K_KEYWORD_DO << "\t";
                cout << "{keyword do}" << "\t";
                break;
            case K_KEYWORD_DOUBLE:
                cout << K_KEYWORD_DOUBLE << "\t";
                cout << "{keyword double}" << "\t";
                break;
            case K_KEYWORD_ELSE:
                cout << K_KEYWORD_ELSE << "\t";
                cout << "{keyword else}" << "\t";
                break;
            case K_KEYWORD_EXTENDS:
                cout << K_KEYWORD_EXTENDS << "\t";
                cout << "{keyword extends}" << "\t";
                break;
            case K_KEYWORD_FALSE:
                cout << K_KEYWORD_FALSE << "\t";
                cout << "{keyword false}" << "\t";
                break;
            case K_KEYWORD_FINAL:
                cout << K_KEYWORD_FINAL << "\t";
                cout << "{keyword final}" << "\t";
                break;
            case K_KEYWORD_FINALLY:
                cout << K_KEYWORD_FINALLY << "\t";
                cout << "{keyword finally}" << "\t";
                break;
            case K_KEYWORD_FLOAT:
                cout << K_KEYWORD_FLOAT << "\t";
                cout << "{keyword float}" << "\t";
                break;
            case K_KEYWORD_FOR:
                cout << K_KEYWORD_FOR << "\t";
                cout << "{keyword for}" << "\t";
                break;
            case K_KEYWORD_GOTO:
                cout << K_KEYWORD_GOTO << "\t";
                cout << "{keyword goto}" << "\t";
                break;
            case K_KEYWORD_IF:
                cout << K_KEYWORD_IF << "\t";
                cout << "{keyword if}" << "\t";
                break;
            case K_KEYWORD_IMPLEMENTS:
                cout << K_KEYWORD_IMPLEMENTS << "\t";
                cout << "{keyword implements}" << "\t";
                break;
            case K_KEYWORD_IMPORT:
                cout << K_KEYWORD_IMPORT << "\t";
                cout << "{keyword import}" << "\t";
                break;
            case K_KEYWORD_INSTANCEOF:
                cout << K_KEYWORD_INSTANCEOF << "\t";
                cout << "{keyword instanceof}" << "\t";
                break;
            case K_KEYWORD_INT:
                cout << K_KEYWORD_INT << "\t";
                cout << "{keyword int}" << "\t";
                break;
            case K_KEYWORD_INTERFACE:
                cout << K_KEYWORD_INTERFACE << "\t";
                cout << "{keyword interface}" << "\t";
                break;
            case K_KEYWORD_LONG:
                cout << K_KEYWORD_LONG << "\t";
                cout << "{keyword long}" << "\t";
                break;
            case K_KEYWORD_NATIVE:
                cout << K_KEYWORD_NATIVE << "\t";
                cout << "{keyword native}" << "\t";
                break;
            case K_KEYWORD_NEW:
                cout << K_KEYWORD_NEW << "\t";
                cout << "{keyword new}" << "\t";
                break;
            case K_KEYWORD_NULL:
                cout << K_KEYWORD_NULL << "\t";
                cout << "{keyword null}" << "\t";
                break;
            case K_KEYWORD_PACKAGE:
                cout << K_KEYWORD_PACKAGE << "\t";
                cout << "{keyword package}" << "\t";
                break;
            case K_KEYWORD_PRIVATE:
                cout << K_KEYWORD_PRIVATE << "\t";
                cout << "{keyword private}" << "\t";
                break;
            case K_KEYWORD_PROTECTED:
                cout << K_KEYWORD_PROTECTED << "\t";
                cout << "{keyword protected}" << "\t";
                break;
            case K_KEYWORD_PUBLIC:
                cout << K_KEYWORD_PUBLIC << "\t";
                cout << "{keyword public}" << "\t";
                break;
            case K_KEYWORD_RETURN:
                cout << K_KEYWORD_RETURN << "\t";
                cout << "{keyword return}" << "\t";
                break;
            case K_KEYWORD_SHORT:
                cout << K_KEYWORD_SHORT << "\t";
                cout << "{keyword shot}" << "\t";
                break;
            case K_KEYWORD_STATIC:
                cout << K_KEYWORD_STATIC << "\t";
                cout << "{keyword static}" << "\t";
                break;
            case K_KEYWORD_SUPER:
                cout << K_KEYWORD_SUPER << "\t";
                cout << "{keyword super}" << "\t";
                break;
            case K_KEYWORD_SWITCH:
                cout << K_KEYWORD_SWITCH << "\t";
                cout << "{keyword switch}" << "\t";
                break;
            case K_KEYWORD_SYNCHRONIZED:
                cout << K_KEYWORD_SYNCHRONIZED << "\t";
                cout << "{keyword synchronized}" << "\t";
                break;
            case K_KEYWORD_THIS:
                cout << K_KEYWORD_THIS << "\t";
                cout << "{keyword this}" << "\t";
                break;
            case K_KEYWORD_THROW:
                cout << K_KEYWORD_THROW << "\t";
                cout << "{keyword throw}" << "\t";
                break;
            case K_KEYWORD_THROWS:
                cout << K_KEYWORD_THROWS << "\t";
                cout << "{keyword throws}" << "\t";
                break;
            case K_KEYWORD_TRANSIENT:
                cout << K_KEYWORD_TRANSIENT << "\t";
                cout << "{keyword transient}" << "\t";
                break;
            case K_KEYWORD_TRUE:
                cout << K_KEYWORD_TRUE << "\t";
                cout << "{keyword true}" << "\t";
                break;
            case K_KEYWORD_TRY:
                cout << K_KEYWORD_TRY << "\t";
                cout << "{keyword try}" << "\t";
                break;
            case K_KEYWORD_VOID:
                cout << K_KEYWORD_VOID << "\t";
                cout << "{keyword void}" << "\t";
                break;
            case K_KEYWORD_VOLATILE:
                cout << K_KEYWORD_VOLATILE << "\t";
                cout << "{keyword volatile}" << "\t";
                break;
            case K_KEYWORD_WHILE:
                cout << K_KEYWORD_WHILE << "\t";
                cout << "{keyword while}" << "\t";
                break;
            case K_IDENTITY:
                cout << K_IDENTITY << "\t";
                cout << "{identity}" << "\t";
                break;
            case K_SPACE:
                cout << K_SPACE<< "\t";
                cout << "{space}" << "\t";
                break;
            case K_CONSTANT_INTEGER:
                cout << K_CONSTANT_INTEGER << "\t";
                cout << "{constant integer}" << "\t";
                break;
            case K_CONSTANT_REAL:
                cout << K_CONSTANT_REAL << "\t";
                cout << "{constant real}" << "\t";
                break;
            case K_CONSTANT_POSITIVE_INTEGER:
                cout << K_CONSTANT_POSITIVE_INTEGER << "\t";
                cout << "{constant positive integer}" << "\t";
                break;
            case K_CONSTANT_NEGATIVE_INTEGER:
                cout << K_CONSTANT_NEGATIVE_INTEGER << "\t";
                cout << "{constant negative integer}" << "\t";
                break;
            case K_CONSTANT_POSITIVE_REAL:
                cout << K_CONSTANT_POSITIVE_REAL << "\t";
                cout << "{constant positive real}" << "\t";
                break;
            case K_CONSTANT_NEGATIVE_REAL:
                cout << K_CONSTANT_NEGATIVE_REAL << "\t";
                cout << "{constant negative real}" << "\t";
                break;
            case K_CONSTANT_BOOL:
                cout << K_CONSTANT_BOOL << "\t";
                cout << "{constant bool}" << "\t";
                break;
            case K_CONSTANT_STRING:
                cout << K_CONSTANT_STRING << "\t";
                cout << "{constant string}" << "\t";
                break;
            case K_CONSTANT_CHAR:
                cout << K_CONSTANT_CHAR << "\t";
                cout << "{costant char}" << "\t";
                break;
            case K_CONSTANT_OCT_INTEGER:
                cout << K_CONSTANT_OCT_INTEGER << "\t";
                cout << "{constant oct integer}" << "\t";
                break;
            case K_CONSTANT_HEX_INTEGER:
                cout << K_CONSTANT_HEX_INTEGER << "\t";
                cout << "{constant hex integer}" << "\t";
                break;
            case K_ESCAPE_OCT_CHAR:
                cout << K_ESCAPE_OCT_CHAR << "\t";
                cout << "{escape oct char}" << "\t";
                break;
            case K_ESCAPE_HEX_CHAR:
                cout << K_ESCAPE_HEX_CHAR << "\t";
                cout << "{escape hex char}" << "\t";
                break;
            case K_ESCAPE_SINGLE_QUO:
                cout << K_ESCAPE_SINGLE_QUO << "\t";
                cout << "{escape single quo}" << "\t";
                break;
            case K_ESCAPE_BACK_SLASH:
                cout << K_ESCAPE_BACK_SLASH << "\t";
                cout << "{escape back slash}" << "\t";
                break;
            case K_ESCAPE_CR:
                cout << K_ESCAPE_CR << "\t";
                cout << "{escape cr}" << "\t";
                break;
            case K_ESCAPE_LF:
                cout << K_ESCAPE_LF << "\t";
                cout << "{escape lf}" << "\t";
                break;
            case K_ESCAPE_FF:
                cout << K_ESCAPE_FF << "\t";
                cout << "{escape ff}" << "\t";
                break;
            case K_ESCAPE_TAB:
                cout << K_ESCAPE_TAB << "\t";
                cout << "{escape tab}" << "\t";
                break;
            case K_ESCAPE_BACKSPACE:
                cout << K_ESCAPE_BACKSPACE << "\t";
                cout << "{escape backspace}" << "\t";
                break;
            case K_BOUNDARY_BRACES_LEFT:
                cout << K_BOUNDARY_BRACES_RIGHT << "\t";
                cout << "{boundary braces right}" << "\t";
                break;
            case K_BOUNDARY_BRACKET_LEFT:
                cout << K_BOUNDARY_BRACKET_LEFT << "\t";
                cout << "{boundary bracket left}" << "\t";
                break;
            case K_BOUNDARY_BRACKET_RIGHT:
                cout << K_BOUNDARY_BRACKET_RIGHT << "\t";
                cout << "{boundary bracket right}" << "\t";
                break;
            case K_BOUNDARY_PARENTHESES_LEFT:
                cout << K_BOUNDARY_PARENTHESES_LEFT << "\t";
                cout << "{boundary parentheses left}" << "\t";
                break;
            case K_BOUNDARY_PARENTHESES_RIGHT:
                cout << K_BOUNDARY_PARENTHESES_RIGHT << "\t";
                cout << "{boundary parenthese right}" << "\t";
                break;
            case K_BOUNDARY_COMMA:
                cout << K_BOUNDARY_COMMA << "\t";
                cout << "{boundary comma}" << "\t";
                break;
            case K_BOUNDARY_PERIOD:
                cout << K_BOUNDARY_PERIOD << "\t";
                cout << "{boundary period}" << "\t";
                break;
            case K_BOUNDARY_SEMICOLON:
                cout << K_BOUNDARY_SEMICOLON << "\t";
                cout << "{boundary semicolon}" << "\t";
                break;
            case K_BOUNDARY:
                cout << K_BOUNDARY << "\t";
                cout << "{boundary}" << "\t";
                break;
            case K_OPR_ASSIGN:
                cout << K_OPR_ASSIGN << "\t";
                cout << "{opr assign}" << "\t";
                break;
            case K_OPR_ADD:
                cout << K_OPR_ADD << "\t";
                cout << "{opr add}" << "\t";
                break;
            case K_OPR_SUB:
                cout << K_OPR_SUB << "\t";
                cout << "{opr sub}" << "\t";
                break;
            case K_OPR_MUL:
                cout << K_OPR_MUL << "\t";
                cout << "{opr mul}" << "\t";
                break;
            case K_OPR_DIV:
                cout << K_OPR_DIV << "\t";
                cout << "{opr div}" << "\t";
                break;
            case K_OPR_MOD:
                cout << K_OPR_MOD << "\t";
                cout << "{opr mod}" << "\t";
                break;
            case K_OPR_GT:
                cout << K_OPR_GT << "\t";
                cout << "{opr gt}" << "\t";
                break;
            case K_OPR_GE:
                cout << K_OPR_GE << "\t";
                cout << "{opr ge}" << "\t";
                break;
            case K_OPR_EQU:
                cout << K_OPR_EQU << "\t";
                cout << "{opr equ}" << "\t";
                break;
            case K_OPR_NEQU:
                cout << K_OPR_NEQU << "\t";
                cout << "{opr nequ}" << "\t";
                break;
            case K_OPR_LT:
                cout << K_OPR_LT << "\t";
                cout << "{opr lt}" << "\t";
                break;
            case K_OPR_LE:
                cout << K_OPR_LE << "\t";
                cout << "{opr le}" << "\t";
                break;
            case K_OPR_AND:
                cout << K_OPR_AND << "\t";
                cout << "{opr and}" << "\t";
                break;
            case K_OPR_OR:
                cout << K_OPR_OR << "\t";
                cout << "{opr or}" << "\t";
                break;
            case K_OPR_NOT:
                cout << K_OPR_NOT << "\t";
                cout << "{opr not}" << "\t";
                break;
            case K_OPR_AND_BIT:
                cout << K_OPR_AND_BIT << "\t";
                cout << "{opr and bit}" << "\t";
                break;
            case K_OPR_OR_BIT:
                cout << K_OPR_OR_BIT << "\t";
                cout << "{opr or bit}" << "\t";
                break;
            case K_OPR_XOR_BIT:
                cout << K_OPR_XOR_BIT << "\t";
                cout << "{opr xor bit}" << "\t";
                break;
            case K_OPR_NOT_BIT:
                cout << K_OPR_NOT_BIT << "\t";
                cout << "{opr not bit}" << "\t";
                break;
            case K_OPR_ADD_ASSIGN:
                cout << K_OPR_ADD_ASSIGN << "\t";
                cout << "{opr add assign}" << "\t";
                break;
            case K_OPR_SUB_ASSIGN:
                cout << K_OPR_SUB_ASSIGN << "\t";
                cout << "{opr sub assign}" << "\t";
                break;
            case K_OPR_MUL_ASSIGN:
                cout << K_OPR_MUL_ASSIGN << "\t";
                cout << "{opr mul assign}" << "\t";
                break;
            case K_OPR_DIV_ASSIGN:
                cout << K_OPR_DIV_ASSIGN << "\t";
                cout << "{opr div assign}" << "\t";
                break;
            case K_OPR_MOD_ASSIGN:
                cout << K_OPR_MOD_ASSIGN << "\t";
                cout << "{opr mod assign}" << "\t";
                break;
            case K_OPR_INC:
                cout << K_OPR_INC << "\t";
                cout << "{opr inc}" << "\t";
                break;
            case K_OPR_DEC:
                cout << K_OPR_DEC << "\t";
                cout << "{opr dec}" << "\t";
                break;
            case K_OPR_AND_ASSIGN:
                cout << K_OPR_AND_ASSIGN << "\t";
                cout << "{opr and assign}" << "\t";
                break;
            case K_OPR_OR_ASSIGN:
                cout << K_OPR_OR_ASSIGN << "\t";
                cout << "{opr or assign}" << "\t";
                break;
            case K_OPR_NOT_ASSIGN:
                cout << K_OPR_NOT_ASSIGN << "\t";
                cout << "{opr not assign}" << "\t";
                break;
            case K_OPR_SHIFT_LEFT:
                cout << K_OPR_SHIFT_LEFT << "\t";
                cout << "{opr shift left}" << "\t";
                break;
            case K_OPR_SHIFT_RIGHT:
                cout << K_OPR_SHIFT_RIGHT << "\t";
                cout << "{opr shift right}" << "\t";
                break;
            case K_OPR_SHIFT_ZERO:
                cout << K_OPR_SHIFT_ZERO << "\t";
                cout << "{opr shift zero}" << "\t";
                break;
            case K_OPR_SHIFT_LEFT_ASSIGN:
                cout << K_OPR_SHIFT_LEFT_ASSIGN << "\t";
                cout << "{opr shift left assign}" << "\t";
                break;
            case K_OPR_SHIFT_RIGHT_ASSIGN:
                cout << K_OPR_SHIFT_RIGHT_ASSIGN << "\t";
                cout << "{ior shift right assign}" << "\t";
                break;
            case K_OPR_SHIFT_ZERO_ASSIGN:
                cout << K_OPR_SHIFT_ZERO_ASSIGN << "\t";
                cout << "{opr shift zero assign}" << "\t";
                break;
            case K_OPR_TERNARY_QUESTION:
                cout << K_OPR_TERNARY_QUESTION << "\t";
                cout << "{opr ternary question}" << "\t";
                break;
            case K_OPR_TERNARY_COLON:
                cout << K_OPR_TERNARY_COLON << "\t";
                cout << "{opt ternary colon}" << "\t";
                break;
            case K_OPR:
                cout << K_OPR << "\t";
                cout << "{opr}" << "\t";
                break;
            default:break;
        }
        cout << tmp.getStr_RealString() << endl;
        of_OutFile << tmp.getStr_RealString() << endl;
    }
}

bool Scanner::_ScanNumber() {
    char ch = c_CurChar;
    Token tok;
    string tmpStr;
    tmpStr.clear();
    tmpStr = ch;
    Position stPos(i_Line, i_Col);
    if (ch == '0') {
        //以0开头，需要考虑实型，整型16进制，整型八进制
        ch = _ReadChar();
        if (ch == EOF || (ch == 0 && b_EndTag)) {
            b_TrueEndTag = true;
            Position edPos(i_Line, i_Col);
            tok.Set(A_LEX_ERROR, K_CONSTANT_INTEGER, tmpStr, stPos, edPos);
            v_Tokens.push_back(tok);
            _LexError(103, tok);
            return false;
        }
        if (ch == '.') {
            //0.继续读
            tmpStr += ch;
            i_Col++;
            ch = _ReadChar();
            if (ch == EOF || (ch == 0 && b_EndTag)) {
                b_TrueEndTag = true;
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_ERROR, K_CONSTANT_REAL, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
                _LexError(103, tok);
                return false;
            }
            if (!(ch >= '0' && ch <= '9')) {
                if (ch == 'E' || ch == 'e') {
                    //0.E继续读
                    tmpStr += ch;
                    i_Col++;
                    ch = _ReadChar();
                    if (ch == EOF || (ch == 0 && b_EndTag)) {
                        b_TrueEndTag = true;
                        Position edPos(i_Line, i_Col);
                        tok.Set(A_LEX_ERROR, K_CONSTANT_REAL, tmpStr, stPos, edPos);
                        v_Tokens.push_back(tok);
                        _LexError(103, tok);
                        return false;
                    }
                    if (ch == '-') {
                        //E后面很可能跟了一个负号
                        tmpStr += ch;
                        i_Col++;
                        ch = _ReadChar();
                        if (ch == EOF || (ch == 0 && b_EndTag)) {
                            b_TrueEndTag = true;
                            Position edPos(i_Line, i_Col);
                            tok.Set(A_LEX_ERROR, K_CONSTANT_REAL, tmpStr, stPos, edPos);
                            v_Tokens.push_back(tok);
                            _LexError(103, tok);
                            return false;
                        }
                    }
                    if ((!(ch >= '0' && ch <= '9'))) {
                        //0.E后面不是数字，出错
                        _Untread();
                        Position edPos(i_Line, i_Col);
                        tok.Set(A_LEX_ERROR, K_CONSTANT_REAL, tmpStr, stPos, edPos);
                        v_Tokens.push_back(tok);
                        _LexError(100, tok);
                        return false;
                    }
                    else {
                        while (ch >= '0' && ch <= '9') {
                            tmpStr += ch;
                            i_Col++;
                            ch = _ReadChar();
                            if (ch == EOF || (ch == 0 && b_EndTag)) {
                                b_TrueEndTag = true;
                                Position edPos(i_Line, i_Col);
                                tok.Set(A_LEX_ERROR, K_CONSTANT_REAL, tmpStr, stPos, edPos);
                                v_Tokens.push_back(tok);
                                _LexError(103, tok);
                                return false;
                            }
                        }
                        if (ch == 'F' || ch == 'f' || ch == 'D' || ch == 'd') {
                            //0.E23F
                            tmpStr += ch;
                            i_Col++;
                            Position edPos(i_Line, i_Col);
                            tok.Set(A_LEX_REAL, K_CONSTANT_REAL, tmpStr, stPos, edPos);
                            v_Tokens.push_back(tok);
                            return true;
                        }
                        else {
                            //0.E23
                            _Untread();
                            Position edPos(i_Line, i_Col);
                            tok.Set(A_LEX_REAL, K_CONSTANT_REAL, tmpStr, stPos, edPos);
                            v_Tokens.push_back(tok);
                            return true;
                        }
                    }
                }
                else if (ch == 'F' || ch == 'f' || ch == 'D' || ch == 'd') {
                    //0.F
                    tmpStr += ch;
                    i_Col++;
                    Position edPos(i_Line, i_Col);
                    tok.Set(A_LEX_REAL, K_CONSTANT_REAL, tmpStr, stPos, edPos);
                    v_Tokens.push_back(tok);
                    return true;
                }
                else {
                    //0.
                    _Untread();
                    Position edPos(i_Line, i_Col);
                    tok.Set(A_LEX_REAL, K_CONSTANT_REAL, tmpStr, stPos, edPos);
                    v_Tokens.push_back(tok);
                    return true;
                }
            }
            else {
                //0.2继续读
                while (ch >= '0' && ch <= '9') {
                    tmpStr += ch;
                    i_Col++;
                    ch = _ReadChar();
                    if (ch == EOF || (ch == 0 && b_EndTag)) {
                        b_TrueEndTag = true;
                        Position edPos(i_Line, i_Col);
                        tok.Set(A_LEX_ERROR, K_CONSTANT_REAL, tmpStr, stPos, edPos);
                        v_Tokens.push_back(tok);
                        _LexError(103, tok);
                        return false;
                    }
                }
                if (ch == 'E' || ch == 'e') {
                    //0.2E继续读
                    tmpStr += ch;
                    i_Col++;
                    ch = _ReadChar();
                    if (ch == EOF || (ch == 0 && b_EndTag)) {
                        b_TrueEndTag = true;
                        Position edPos(i_Line, i_Col);
                        tok.Set(A_LEX_ERROR, K_CONSTANT_REAL, tmpStr, stPos, edPos);
                        v_Tokens.push_back(tok);
                        _LexError(103, tok);
                        return false;
                    }
                    if (ch == '-') {
                        //E后面很可能跟了一个负号
                        tmpStr += ch;
                        i_Col++;
                        ch = _ReadChar();
                        if (ch == EOF || (ch == 0 && b_EndTag)) {
                            b_TrueEndTag = true;
                            Position edPos(i_Line, i_Col);
                            tok.Set(A_LEX_ERROR, K_CONSTANT_REAL, tmpStr, stPos, edPos);
                            v_Tokens.push_back(tok);
                            _LexError(103, tok);
                            return false;
                        }
                    }
                    if (!(ch >= '0' && ch <= '9')) {
                        //0.E后面不是数字，出错
                        if (ch == 'F' || ch == 'f' || ch == 'D' || ch == 'd') {
                            tmpStr += ch;
                            i_Col++;
                        }
                        else {
                            _Untread();
                        }
                        Position edPos(i_Line, i_Col);
                        tok.Set(A_LEX_ERROR, K_CONSTANT_REAL, tmpStr, stPos, edPos);
                        v_Tokens.push_back(tok);
                        _LexError(100, tok);
                        return false;
                    }
                    else {
                        while (ch >= '0' && ch <= '9') {
                            tmpStr += ch;
                            i_Col++;
                            ch = _ReadChar();
                            if (ch == EOF || (ch == 0 && b_EndTag)) {
                                b_TrueEndTag = true;
                                Position edPos(i_Line, i_Col);
                                tok.Set(A_LEX_ERROR, K_CONSTANT_REAL, tmpStr, stPos, edPos);
                                v_Tokens.push_back(tok);
                                _LexError(103, tok);
                                return false;
                            }
                        }
                        if (ch == 'F' || ch == 'f' || ch == 'D' || ch == 'd') {
                            //0.2E23F
                            tmpStr += ch;
                            i_Col++;
                            Position edPos(i_Line, i_Col);
                            tok.Set(A_LEX_REAL, K_CONSTANT_REAL, tmpStr, stPos, edPos);
                            v_Tokens.push_back(tok);
                            return true;
                        }
                        else {
                            //0.2E23
                            _Untread();
                            Position edPos(i_Line, i_Col);
                            tok.Set(A_LEX_REAL, K_CONSTANT_REAL, tmpStr, stPos, edPos);
                            v_Tokens.push_back(tok);
                            return true;
                        }
                    }
                }
                else if (ch == 'F' || ch == 'f' || ch == 'D' || ch == 'd') {
                    //0.2F
                    tmpStr += ch;
                    i_Col++;
                    Position edPos(i_Line, i_Col);
                    tok.Set(A_LEX_REAL, K_CONSTANT_REAL, tmpStr, stPos, edPos);
                    v_Tokens.push_back(tok);
                    return true;
                }
                else {
                    //0.2
                    _Untread();
                    Position edPos(i_Line, i_Col);
                    tok.Set(A_LEX_REAL, K_CONSTANT_REAL, tmpStr, stPos, edPos);
                    v_Tokens.push_back(tok);
                    return true;
                }
            }
        }
        else if (ch == 'E' || ch == 'e') {
            //0E继续读
            tmpStr += ch;
            i_Col++;
            ch = _ReadChar();
            if (ch == EOF || (ch == 0 && b_EndTag)) {
                b_TrueEndTag = true;
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_ERROR, K_CONSTANT_REAL, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
                _LexError(103, tok);
                return false;
            }
            if (ch == '-') {
                //E后面很可能跟了一个负号
                tmpStr += ch;
                i_Col++;
                ch = _ReadChar();
                if (ch == EOF || (ch == 0 && b_EndTag)) {
                    b_TrueEndTag = true;
                    Position edPos(i_Line, i_Col);
                    tok.Set(A_LEX_ERROR, K_CONSTANT_REAL, tmpStr, stPos, edPos);
                    v_Tokens.push_back(tok);
                    _LexError(103, tok);
                    return false;
                }
            }
            if (!(ch >= '0' && ch <= '9')) {
                //E后面没有数字，出错
                if (ch == 'F' || ch == 'f' || ch == 'D' || ch == 'd') {
                    tmpStr += ch;
                    i_Col++;
                }
                else {
                    _Untread();
                }
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_ERROR, K_CONSTANT_REAL, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
                _LexError(100, tok);
                return false;
            }
            while ((ch >= '0' && ch <= '9')) {
                tmpStr += ch;
                i_Col++;
                ch = _ReadChar();
                if (ch == EOF || (ch == 0 && b_EndTag)) {
                    b_TrueEndTag = true;
                    Position edPos(i_Line, i_Col);
                    tok.Set(A_LEX_ERROR, K_CONSTANT_REAL, tmpStr, stPos, edPos);
                    v_Tokens.push_back(tok);
                    _LexError(103, tok);
                    return false;
                }
            }
            if (ch == 'F' || ch == 'f' || ch == 'D' || ch == 'd') {
                //0E2f
                tmpStr += ch;
                i_Col++;
            }
            else {
                //0E1
                _Untread();
            }
            Position edPos(i_Line, i_Col);
            tok.Set(A_LEX_REAL, K_CONSTANT_REAL, tmpStr, stPos, edPos);
            v_Tokens.push_back(tok);
            return true;
        }
        else if (ch == 'F' || ch == 'f' || ch == 'D' || ch == 'd') {
            //0F
            tmpStr += ch;
            i_Col++;
            Position edPos(i_Line, i_Col);
            tok.Set(A_LEX_REAL, K_CONSTANT_REAL, tmpStr, stPos, edPos);
            v_Tokens.push_back(tok);
            return true;
        }
        else if (ch == 'L' || ch == 'l') {
            //0L
            tmpStr += ch;
            i_Col++;
            Position edPos(i_Line, i_Col);
            tok.Set(A_LEX_INT, K_CONSTANT_INTEGER, tmpStr, stPos, edPos);
            v_Tokens.push_back(tok);
            return true;
        }
        else if (ch == 'x') {
            //0x十六进制，继续读
            tmpStr += ch;
            i_Col++;
            ch = _ReadChar();
            if (ch == EOF || (ch == 0 && b_EndTag)) {
                b_TrueEndTag = true;
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_ERROR, K_CONSTANT_HEX_INTEGER, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
                _LexError(103, tok);
                return false;
            }
            if (!((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'F') || (ch >= 'a' && ch <= 'f'))) {
                //0x后面不是数字，出错
                if (ch == 'F' || ch == 'f' || ch == 'D' || ch == 'd') {
                    //0xF
                    tmpStr += ch;
                    i_Col++;
                }
                else {
                    //0x
                    _Untread();
                }
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_ERROR, K_CONSTANT_HEX_INTEGER, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
                _LexError(101, tok);
                return false;
            }
            else {
                while ((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'F') || (ch >= 'a' && ch <= 'f')) {
                    tmpStr += ch;
                    i_Col++;
                    ch = _ReadChar();
                    if (ch == EOF || (ch == 0 && b_EndTag)) {
                        b_TrueEndTag = true;
                        Position edPos(i_Line, i_Col);
                        tok.Set(A_LEX_ERROR, K_CONSTANT_HEX_INTEGER, tmpStr, stPos, edPos);
                        v_Tokens.push_back(tok);
                        _LexError(103, tok);
                        return false;
                    }
                }
                if (ch == 'L' || ch == 'l') {
                    //0x123L
                    tmpStr += ch;
                    i_Col++;
                }
                else {
                    //0x123
                    _Untread();
                }
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_INT, K_CONSTANT_HEX_INTEGER, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
                return true;
            }
        }
        else if (ch >= '0' && ch <= '9') {
            //准备判断八进制
            bool octFlag = ch >= '0' && ch <= '7';;//当前的字符串是否仍符合八进制
            tmpStr += ch;
            i_Col++;
            ch = _ReadChar();
            if (ch == EOF || (ch == 0 && b_EndTag)) {
                b_TrueEndTag = true;
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_ERROR, K_CONSTANT_OCT_INTEGER, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
                _LexError(103, tok);
                return false;
            }
            while (ch >= '0' && ch <= '9') {
                if (octFlag) {
                    octFlag = ch >= '0' && ch <= '7';
                }
                tmpStr += ch;
                i_Col++;
                ch = _ReadChar();
                if (ch == EOF || (ch == 0 && b_EndTag)) {
                    b_TrueEndTag = true;
                    Position edPos(i_Line, i_Col);
                    tok.Set(A_LEX_ERROR, K_CONSTANT_OCT_INTEGER, tmpStr, stPos, edPos);
                    v_Tokens.push_back(tok);
                    _LexError(103, tok);
                    return false;
                }
            }
            if (ch == 'F' || ch == 'f' || ch == 'D' || ch == 'd') {
                //这个时候认为是十进制实型
                tmpStr += ch;
                i_Col++;
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_REAL, K_CONSTANT_REAL, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
                return true;
            }
            else {
                //这个时候认为是八进制
                _Untread();
                Position edPos(i_Line, i_Col);
                if (octFlag) {
                    tok.Set(A_LEX_INT, K_CONSTANT_OCT_INTEGER, tmpStr, stPos, edPos);
                    v_Tokens.push_back(tok);
                    return true;
                }
                else {
                    //不再符合八进制
                    tok.Set(A_LEX_ERROR, K_CONSTANT_OCT_INTEGER, tmpStr, stPos, edPos);
                    v_Tokens.push_back(tok);
                    _LexError(102, tok);
                    return false;
                }
            }
        }
        else {
            _Untread();
            Position edPos(i_Line, i_Col);
            tok.Set(A_LEX_INT, K_CONSTANT_INTEGER, tmpStr, stPos, edPos);
            v_Tokens.push_back(tok);
            return true;
        }
    }
    else {
        //以其他数字开头
        ch = _ReadChar();
        if (ch == EOF || (ch == 0 && b_EndTag)) {
            b_TrueEndTag = true;
            Position edPos(i_Line, i_Col);
            tok.Set(A_LEX_ERROR, K_CONSTANT_INTEGER, tmpStr, stPos, edPos);
            v_Tokens.push_back(tok);
            _LexError(103, tok);
            return false;
        }
        while ((ch >= '0' && ch <= '9')) {
            tmpStr += ch;
            i_Col++;
            ch = _ReadChar();
            if (ch == EOF || (ch == 0 && b_EndTag)) {
                b_TrueEndTag = true;
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_ERROR, K_CONSTANT_INTEGER, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
                _LexError(103, tok);
                return false;
            }
        }
        //跳出循环，如果当前字符是小数点或者F，f，D，d，按照实型常量判断，否则按照整型常量处理
        if (ch == '.') {
            tmpStr += ch;
            i_Col++;
            //继续读
            ch = _ReadChar();
            if (ch == EOF || (ch == 0 && b_EndTag)) {
                b_TrueEndTag = true;
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_ERROR, K_CONSTANT_REAL, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
                _LexError(103, tok);
                return false;
            }
            while ((ch >= '0' && ch <= '9')) {
                tmpStr += ch;
                i_Col++;
                ch = _ReadChar();
                if (ch == EOF || (ch == 0 && b_EndTag)) {
                    b_TrueEndTag = true;
                    Position edPos(i_Line, i_Col);
                    tok.Set(A_LEX_ERROR, K_CONSTANT_REAL, tmpStr, stPos, edPos);
                    v_Tokens.push_back(tok);
                    _LexError(103, tok);
                    return false;
                }
            }
            if (ch == 'F' || ch == 'f' || ch == 'D' || ch == 'd') {
                tmpStr += ch;
                i_Col++;
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_REAL, K_CONSTANT_REAL, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
                return true;
            }
            else if (ch == 'E' || ch == 'e') {
                //1.32E继续读
                tmpStr += ch;
                i_Col++;
                ch = _ReadChar();
                if (ch == EOF || (ch == 0 && b_EndTag)) {
                    b_TrueEndTag = true;
                    Position edPos(i_Line, i_Col);
                    tok.Set(A_LEX_ERROR, K_CONSTANT_REAL, tmpStr, stPos, edPos);
                    v_Tokens.push_back(tok);
                    _LexError(103, tok);
                    return false;
                }
                if (ch == '-') {
                    //E后面很可能跟了一个负号
                    tmpStr += ch;
                    i_Col++;
                    ch = _ReadChar();
                    if (ch == EOF || (ch == 0 && b_EndTag)) {
                        b_TrueEndTag = true;
                        Position edPos(i_Line, i_Col);
                        tok.Set(A_LEX_ERROR, K_CONSTANT_REAL, tmpStr, stPos, edPos);
                        v_Tokens.push_back(tok);
                        _LexError(103, tok);
                        return false;
                    }
                }
                if (!(ch >= '0' && ch <= '9')) {
                    //E后面没有数字，出错
                    if (ch == 'F' || ch == 'f' || ch == 'D' || ch == 'd') {
                        tmpStr += ch;
                        i_Col++;
                    }
                    else {
                        _Untread();
                    }
                    Position edPos(i_Line, i_Col);
                    tok.Set(A_LEX_ERROR, K_CONSTANT_REAL, tmpStr, stPos, edPos);
                    v_Tokens.push_back(tok);
                    _LexError(100, tok);
                    return false;
                }
                while ((ch >= '0' && ch <= '9')) {
                    tmpStr += ch;
                    i_Col++;
                    ch = _ReadChar();
                    if (ch == EOF || (ch == 0 && b_EndTag)) {
                        b_TrueEndTag = true;
                        Position edPos(i_Line, i_Col);
                        tok.Set(A_LEX_ERROR, K_CONSTANT_REAL, tmpStr, stPos, edPos);
                        v_Tokens.push_back(tok);
                        _LexError(103, tok);
                        return false;
                    }
                }
                if (ch == 'F' || ch == 'f' || ch == 'D' || ch == 'd') {
                    tmpStr += ch;
                    i_Col++;
                }
                else {
                    _Untread();
                }
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_REAL, K_CONSTANT_REAL, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
                return true;
            }
            else {
                _Untread();
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_REAL, K_CONSTANT_REAL, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
                return true;
            }
        }
        else if (ch == 'E' || ch == 'e') {
            //123E继续读
            tmpStr += ch;
            i_Col++;
            ch = _ReadChar();
            if (ch == EOF || (ch == 0 && b_EndTag)) {
                b_TrueEndTag = true;
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_ERROR, K_CONSTANT_REAL, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
                _LexError(103, tok);
                return false;
            }
            if (ch == '-') {
                //E后面很可能跟了一个负号
                tmpStr += ch;
                i_Col++;
                ch = _ReadChar();
                if (ch == EOF || (ch == 0 && b_EndTag)) {
                    b_TrueEndTag = true;
                    Position edPos(i_Line, i_Col);
                    tok.Set(A_LEX_ERROR, K_CONSTANT_REAL, tmpStr, stPos, edPos);
                    v_Tokens.push_back(tok);
                    _LexError(103, tok);
                    return false;
                }
            }
            if (!(ch >= '0' && ch <= '9')) {
                //E后面没有数字，出错
                if (ch == 'F' || ch == 'f' || ch == 'D' || ch == 'd') {
                    tmpStr += ch;
                    i_Col++;
                }
                else {
                    _Untread();
                }
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_ERROR, K_CONSTANT_REAL, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
                _LexError(100, tok);
                return false;
            }
            while ((ch >= '0' && ch <= '9')) {
                tmpStr += ch;
                i_Col++;
                ch = _ReadChar();
                if (ch == EOF || (ch == 0 && b_EndTag)) {
                    b_TrueEndTag = true;
                    Position edPos(i_Line, i_Col);
                    tok.Set(A_LEX_ERROR, K_CONSTANT_REAL, tmpStr, stPos, edPos);
                    v_Tokens.push_back(tok);
                    _LexError(103, tok);
                    return false;
                }
            }
            if (ch == 'F' || ch == 'f' || ch == 'D' || ch == 'd') {
                tmpStr += ch;
                i_Col++;
            }
            else {
                _Untread();
            }
            Position edPos(i_Line, i_Col);
            tok.Set(A_LEX_REAL, K_CONSTANT_REAL, tmpStr, stPos, edPos);
            v_Tokens.push_back(tok);
            return true;
        }
        else if (ch == 'F' || ch == 'f' || ch == 'D' || ch == 'd') {
            //123F，实型常量
            tmpStr += ch;
            i_Col++;
            Position edPos(i_Line, i_Col);
            tok.Set(A_LEX_REAL, K_CONSTANT_REAL, tmpStr, stPos, edPos);
            v_Tokens.push_back(tok);
            return true;
        }
        else if (ch == 'L' || ch == 'l') {
            //132L，整型常量
            tmpStr += ch;
            i_Col++;
            Position edPos(i_Line, i_Col);
            tok.Set(A_LEX_INT, K_CONSTANT_INTEGER, tmpStr, stPos, edPos);
            v_Tokens.push_back(tok);
            return true;
        }
        else {
            //后面的字符与数字无关，回退之
            _Untread();
            Position edPos(i_Line, i_Col);
            tok.Set(A_LEX_INT, K_CONSTANT_INTEGER, tmpStr, stPos, edPos);
            v_Tokens.push_back(tok);
            return true;
        }
    }

}

void Scanner::_LexError(int errorCode, Token tok) {
    v_Err_Tokens.push_back(tok);
    string err;
    switch (errorCode) {
        case 100:
            err = "实型常量格式错误";
            break;
        case 101:
            err = "十六进制数格式错误";
            break;
        case 102:
            err = "八进制数格式错误";
            break;
        case 103 :
            err = "程序结尾非法";
            break;
        case 104:
            err = "程序行尾非法";
            break;
        case 105:
            err = "字符长度非法";
            break;
        case 106:
            err = "八进制字符格式错误";
            break;
        case 107:
            err = "十六进制字符格式错误";
            break;
        case 108:
            err = "字符单引号格式错误";
            break;
        case 109:
            err = "字符反斜杠格式错误";
            break;
        case 110:
            err = "字符回车格式错误";
            break;
        case 111:
            err = "字符换行格式错误";
            break;
        case 112:
            err = "字符走纸换页格式错误";
            break;
        case 113:
            err = "字符TAB格式错误";
            break;
        case 114:
            err = "字符退格格式错误";
            break;
        case 115:
            err = "转义字符格式错误";
            break;
        case 116:
            err = "空字符";
            break;
        default:
            err = "未知错误";
            break;
    }
    v_Err_String.push_back(err);
}

void Scanner::_ScanCharacter() {
    char ch = c_CurChar;
    Token tok;
    string tmpStr;
    tmpStr.clear();
    tmpStr = ch;
    Position stPos(i_Line, i_Col);
    ch = _ReadChar();
    if (ch == EOF || (ch == 0 && b_EndTag)) {
        b_TrueEndTag = true;
        Position edPos(i_Line, i_Col);
        tok.Set(A_LEX_ERROR, K_CONSTANT_CHAR, tmpStr, stPos, edPos);
        v_Tokens.push_back(tok);
        _LexError(103, tok);
        return;
    }
    if (ch == '\n' || ch == '\t' || ch == '\f' || ch == '\r' || ch == '\b') {
        //转义符，直接报错
        _Untread();
        Position edPos(i_Line, i_Col);
        tok.Set(A_LEX_ERROR, K_CONSTANT_CHAR, tmpStr, stPos, edPos);
        v_Tokens.push_back(tok);
        _LexError(104, tok);
        return;
    }
    else if (ch == '\\') {
        tmpStr += ch;
        i_Col++;
        //准备判断转义符
        ch = _ReadChar();
        if (ch == EOF || (ch == 0 && b_EndTag)) {
            b_TrueEndTag = true;
            Position edPos(i_Line, i_Col);
            tok.Set(A_LEX_ERROR, K_CONSTANT_CHAR, tmpStr, stPos, edPos);
            v_Tokens.push_back(tok);
            _LexError(103, tok);
            return;
        }
        if (ch >= '0' && ch <= '7') {
            //八进制字符
            tmpStr += ch;
            i_Col++;
            bool end = false;
            bool flag = true;
            for (int i = 0; i < 2; ++i) {
                ch = _ReadChar();
                if (ch == EOF || (ch == 0 && b_EndTag)) {
                    b_TrueEndTag = true;
                    Position edPos(i_Line, i_Col);
                    tok.Set(A_LEX_ERROR, K_CONSTANT_CHAR, tmpStr, stPos, edPos);
                    v_Tokens.push_back(tok);
                    _LexError(103, tok);
                    return;
                }
                if (ch == '\n' || ch == '\t' || ch == '\f' || ch == '\r' || ch == '\b') {
                    //转义符，直接报错
                    _Untread();
                    Position edPos(i_Line, i_Col);
                    tok.Set(A_LEX_ERROR, K_CONSTANT_CHAR, tmpStr, stPos, edPos);
                    v_Tokens.push_back(tok);
                    _LexError(104, tok);
                    return;
                }
                tmpStr += ch;
                i_Col++;
                if (ch == '\'') {
                    end = true;
                    break;
                }
                if (flag) {
                    flag = ch >= '0' && ch <= '7';
                }
            }
            if (!end) {
                //向后读了三个字符，其中没有结束
                ch = _ReadChar();
                if (ch == EOF || (ch == 0 && b_EndTag)) {
                    b_TrueEndTag = true;
                    Position edPos(i_Line, i_Col);
                    tok.Set(A_LEX_ERROR, K_CONSTANT_CHAR, tmpStr, stPos, edPos);
                    v_Tokens.push_back(tok);
                    _LexError(103, tok);
                    return;
                }
                if (ch == '\n' || ch == '\t' || ch == '\f' || ch == '\r' || ch == '\b') {
                    //转义符，直接报错
                    _Untread();
                    Position edPos(i_Line, i_Col);
                    tok.Set(A_LEX_ERROR, K_CONSTANT_CHAR, tmpStr, stPos, edPos);
                    v_Tokens.push_back(tok);
                    _LexError(104, tok);
                    return;
                }
                tmpStr += ch;
                i_Col++;
                if (ch == '\'') {
                    //再读读到单引号，可以正确结束
                    Position edPos(i_Line, i_Col);
                    tok.Set(A_LEX_CHAR, K_ESCAPE_OCT_CHAR, tmpStr, stPos, edPos);
                    v_Tokens.push_back(tok);
                    return;
                }
                else {
                    //否则就是错的
                    ch = _ReadChar();
                    if (ch == EOF || (ch == 0 && b_EndTag)) {
                        b_TrueEndTag = true;
                        Position edPos(i_Line, i_Col);
                        tok.Set(A_LEX_ERROR, K_CONSTANT_CHAR, tmpStr, stPos, edPos);
                        v_Tokens.push_back(tok);
                        _LexError(103, tok);
                        return;
                    }
                    while (ch != '\'') {
                        if (ch == '\n' || ch == '\t' || ch == '\f' || ch == '\r' || ch == '\b') {
                            //转义符，直接报错
                            _Untread();
                            Position edPos(i_Line, i_Col);
                            tok.Set(A_LEX_ERROR, K_CONSTANT_CHAR, tmpStr, stPos, edPos);
                            v_Tokens.push_back(tok);
                            _LexError(104, tok);
                            return;
                        }
                        tmpStr += ch;
                        i_Col++;
                        ch = _ReadChar();
                        if (ch == EOF || (ch == 0 && b_EndTag)) {
                            b_TrueEndTag = true;
                            Position edPos(i_Line, i_Col);
                            tok.Set(A_LEX_ERROR, K_CONSTANT_CHAR, tmpStr, stPos, edPos);
                            v_Tokens.push_back(tok);
                            _LexError(103, tok);
                            return;
                        }
                    }
                    tmpStr += ch;
                    i_Col++;
                    Position edPos(i_Line, i_Col);
                    tok.Set(A_LEX_ERROR, K_ESCAPE_OCT_CHAR, tmpStr, stPos, edPos);
                    v_Tokens.push_back(tok);
                    _LexError(106, tok);
                    return;
                }
            }
            else {
                //向后读了三个字符，其中有单引号，提前结束
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_CHAR, K_ESCAPE_OCT_CHAR, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
                return;
            }
        }
        else if (ch == 'u') {
            //十六进制字符
            tmpStr += ch;
            i_Col++;
            bool end = false;
            bool flag = true;
            for (int i = 0; i < 4; ++i) {
                ch = _ReadChar();
                if (ch == EOF || (ch == 0 && b_EndTag)) {
                    b_TrueEndTag = true;
                    Position edPos(i_Line, i_Col);
                    tok.Set(A_LEX_ERROR, K_CONSTANT_CHAR, tmpStr, stPos, edPos);
                    v_Tokens.push_back(tok);
                    _LexError(103, tok);
                    return;
                }
                if (ch == '\n' || ch == '\t' || ch == '\f' || ch == '\r' || ch == '\b') {
                    //转义符，直接报错
                    _Untread();
                    Position edPos(i_Line, i_Col);
                    tok.Set(A_LEX_ERROR, K_CONSTANT_CHAR, tmpStr, stPos, edPos);
                    v_Tokens.push_back(tok);
                    _LexError(104, tok);
                    return;
                }
                tmpStr += ch;
                i_Col++;
                if (ch == '\'') {
                    end = true;
                    break;
                }
                if (flag) {
                    flag = (ch >= '0' && ch <= '7') || (ch >= 'A' && ch <= 'F') || (ch >= 'a' && ch <= 'f');
                }
            }
            if (!end)
            {
                //向后读四个，没结束
                ch = _ReadChar();
                if (ch == EOF || (ch == 0 && b_EndTag)) {
                    b_TrueEndTag = true;
                    Position edPos(i_Line, i_Col);
                    tok.Set(A_LEX_ERROR, K_CONSTANT_CHAR, tmpStr, stPos, edPos);
                    v_Tokens.push_back(tok);
                    _LexError(103, tok);
                    return;
                }
                if (ch == '\n' || ch == '\t' || ch == '\f' || ch == '\r' || ch == '\b') {
                    //转义符，直接报错
                    _Untread();
                    Position edPos(i_Line, i_Col);
                    tok.Set(A_LEX_ERROR, K_CONSTANT_CHAR, tmpStr, stPos, edPos);
                    v_Tokens.push_back(tok);
                    _LexError(104, tok);
                    return;
                }
                tmpStr += ch;
                i_Col++;
                if (ch == '\'') {
                    //再读读到单引号，可以正确结束
                    Position edPos(i_Line, i_Col);
                    tok.Set(A_LEX_CHAR, K_ESCAPE_HEX_CHAR, tmpStr, stPos, edPos);
                    v_Tokens.push_back(tok);
                    return;
                }
                else {
                    //否则就是错的
                    ch = _ReadChar();
                    if (ch == EOF || (ch == 0 && b_EndTag)) {
                        b_TrueEndTag = true;
                        Position edPos(i_Line, i_Col);
                        tok.Set(A_LEX_ERROR, K_CONSTANT_CHAR, tmpStr, stPos, edPos);
                        v_Tokens.push_back(tok);
                        _LexError(103, tok);
                        return;
                    }
                    while (ch != '\'') {
                        if (ch == '\n' || ch == '\t' || ch == '\f' || ch == '\r' || ch == '\b') {
                            //转义符，直接报错
                            _Untread();
                            Position edPos(i_Line, i_Col);
                            tok.Set(A_LEX_ERROR, K_CONSTANT_CHAR, tmpStr, stPos, edPos);
                            v_Tokens.push_back(tok);
                            _LexError(104, tok);
                            return;
                        }
                        tmpStr += ch;
                        i_Col++;
                        ch = _ReadChar();
                        if (ch == EOF || (ch == 0 && b_EndTag)) {
                            b_TrueEndTag = true;
                            Position edPos(i_Line, i_Col);
                            tok.Set(A_LEX_ERROR, K_CONSTANT_CHAR, tmpStr, stPos, edPos);
                            v_Tokens.push_back(tok);
                            _LexError(103, tok);
                            return;
                        }
                    }
                    tmpStr += ch;
                    i_Col++;
                    Position edPos(i_Line, i_Col);
                    tok.Set(A_LEX_ERROR, K_ESCAPE_HEX_CHAR, tmpStr, stPos, edPos);
                    v_Tokens.push_back(tok);
                    _LexError(107, tok);
                    return;
                }
            } else {
                //向后读了四个字符，其中有单引号，提前结束
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_CHAR, K_ESCAPE_HEX_CHAR, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
                return;
            }
        }
        else if (ch == '\'') {
            //判断单引号
            tmpStr += ch;
            i_Col++;
            //读下一个字符，看看是不是单引号，结束
            ch = _ReadChar();
            if (ch == EOF || (ch == 0 && b_EndTag)) {
                b_TrueEndTag = true;
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_ERROR, K_CONSTANT_CHAR, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
                _LexError(103, tok);
                return;
            }
            if (ch == '\n' || ch == '\t' || ch == '\f' || ch == '\r' || ch == '\b') {
                //转义符，直接报错
                _Untread();
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_ERROR, K_ESCAPE_SINGLE_QUO, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
                _LexError(104, tok);
                return;
            }
            if (ch == '\'') {
                //是单引号，结束
                tmpStr += ch;
                i_Col++;
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_CHAR, K_ESCAPE_SINGLE_QUO, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
                return;
            }
            else {
                //不是单引号，就一直向后读，然后报错
                _FillUselessCharacter(108, tmpStr, stPos, ch);
                return;
            }
        }
        else if (ch == '\\') {
            //判断反斜杠
            tmpStr += ch;
            i_Col++;
            //再读一个字符
            ch = _ReadChar();
            if (ch == EOF || (ch == 0 && b_EndTag)) {
                b_TrueEndTag = true;
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_ERROR, K_CONSTANT_CHAR, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
                _LexError(103, tok);
                return;
            }
            if (ch == '\n' || ch == '\t' || ch == '\f' || ch == '\r' || ch == '\b') {
                //转义符，直接报错
                _Untread();
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_ERROR, K_ESCAPE_BACK_SLASH, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
                _LexError(104, tok);
                return;
            }
            if (ch == '\'') {
                tmpStr += ch;
                i_Col++;
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_CHAR, K_ESCAPE_BACK_SLASH, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
                return;
            }
            else {
                _FillUselessCharacter(109, tmpStr, stPos, ch);
                return;
            }
        }
        else if (ch == 'r') {
            //判断\r
            tmpStr += ch;
            i_Col++;
            //再读一个字符
            ch = _ReadChar();
            if (ch == EOF || (ch == 0 && b_EndTag)) {
                b_TrueEndTag = true;
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_ERROR, K_CONSTANT_CHAR, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
                _LexError(103, tok);
                return;
            }
            if (ch == '\n' || ch == '\t' || ch == '\f' || ch == '\r' || ch == '\b') {
                //转义符，直接报错
                _Untread();
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_ERROR, K_ESCAPE_CR, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
                _LexError(104, tok);
                return;
            }
            if (ch == '\'') {
                tmpStr += ch;
                i_Col++;
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_CHAR, K_ESCAPE_CR, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
                return;
            }
            else {
                _FillUselessCharacter(110, tmpStr, stPos, ch);
                return;
            }
        }
        else if (ch == 'n') {
            //判断\n
            tmpStr += ch;
            i_Col++;
            //再读一个字符
            ch = _ReadChar();
            if (ch == EOF || (ch == 0 && b_EndTag)) {
                b_TrueEndTag = true;
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_ERROR, K_CONSTANT_CHAR, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
                _LexError(103, tok);
                return;
            }
            if (ch == '\n' || ch == '\t' || ch == '\f' || ch == '\r' || ch == '\b') {
                //转义符，直接报错
                _Untread();
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_ERROR, K_ESCAPE_LF, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
                _LexError(104, tok);
                return;
            }
            if (ch == '\'') {
                tmpStr += ch;
                i_Col++;
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_CHAR, K_ESCAPE_LF, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
                return;
            }
            else {
                _FillUselessCharacter(111, tmpStr, stPos, ch);
                return;
            }
        }
        else if (ch == 'f') {
            //判断\f
            tmpStr += ch;
            i_Col++;
            //再读一个字符
            ch = _ReadChar();
            if (ch == EOF || (ch == 0 && b_EndTag)) {
                b_TrueEndTag = true;
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_ERROR, K_CONSTANT_CHAR, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
                _LexError(103, tok);
                return;
            }
            if (ch == '\n' || ch == '\t' || ch == '\f' || ch == '\r' || ch == '\b') {
                //转义符，直接报错
                _Untread();
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_ERROR, K_ESCAPE_FF, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
                _LexError(104, tok);
                return;
            }
            if (ch == '\'') {
                tmpStr += ch;
                i_Col++;
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_CHAR, K_ESCAPE_FF, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
                return;
            }
            else {
                _FillUselessCharacter(112, tmpStr, stPos, ch);
                return;
            }
        }
        else if (ch == 't') {
            //判断\t
            tmpStr += ch;
            i_Col++;
            //再读一个字符
            ch = _ReadChar();
            if (ch == EOF || (ch == 0 && b_EndTag)) {
                b_TrueEndTag = true;
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_ERROR, K_CONSTANT_CHAR, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
                _LexError(103, tok);
                return;
            }
            if (ch == '\n' || ch == '\t' || ch == '\f' || ch == '\r' || ch == '\b') {
                //转义符，直接报错
                _Untread();
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_ERROR, K_ESCAPE_TAB, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
                _LexError(104, tok);
                return;
            }
            if (ch == '\'') {
                tmpStr += ch;
                i_Col++;
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_CHAR, K_ESCAPE_FF, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
                return;
            }
            else {
                _FillUselessCharacter(113, tmpStr, stPos, ch);
                return;
            }
        }
        else if (ch == 'b') {
            //判断\b
            tmpStr += ch;
            i_Col++;
            //再读一个字符
            ch = _ReadChar();
            if (ch == EOF || (ch == 0 && b_EndTag)) {
                b_TrueEndTag = true;
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_ERROR, K_CONSTANT_CHAR, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
                _LexError(103, tok);
                return;
            }
            if (ch == '\n' || ch == '\t' || ch == '\f' || ch == '\r' || ch == '\b') {
                //转义符，直接报错
                _Untread();
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_ERROR, K_ESCAPE_BACKSPACE, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
                _LexError(104, tok);
                return;
            }
            if (ch == '\'') {
                tmpStr += ch;
                i_Col++;
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_CHAR, K_ESCAPE_BACKSPACE, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
                return;
            }
            else {
                _FillUselessCharacter(114, tmpStr, stPos, ch);
                return;
            }
        }
        else {
            //非法，报错
            tmpStr += ch;
            i_Col++;
            //再读一个字符
            ch = _ReadChar();
            if (ch == EOF || (ch == 0 && b_EndTag)) {
                b_TrueEndTag = true;
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_ERROR, K_CONSTANT_CHAR, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
                _LexError(103, tok);
                return;
            }
            if (ch == '\n' || ch == '\t' || ch == '\f' || ch == '\r' || ch == '\b') {
                //转义符，直接报错
                _Untread();
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_ERROR, K_CONSTANT_CHAR, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
                _LexError(104, tok);
                return;
            }
            if (ch == '\'') {
                tmpStr += ch;
                i_Col++;
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_ERROR, K_CONSTANT_CHAR, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
                _LexError(115, tok);
                return;
            }
            else {
                _FillUselessCharacter(115, tmpStr, stPos, ch);
                return;
            }
        }
    }
    else if (ch == '\'') {
        //空字符，报错
        _Untread();
        Position edPos(i_Line, i_Col);
        tok.Set(A_LEX_ERROR, K_CONSTANT_CHAR, tmpStr, stPos, edPos);
        v_Tokens.push_back(tok);
        _LexError(116, tok);
        return;
    }
    else {
        //正常的字符，后面跟着一个单引号正常结束，否则报错
        tmpStr += ch;
        i_Col++;
        ch = _ReadChar();
        if (ch == EOF || (ch == 0 && b_EndTag)) {
            b_TrueEndTag = true;
            Position edPos(i_Line, i_Col);
            tok.Set(A_LEX_ERROR, K_CONSTANT_CHAR, tmpStr, stPos, edPos);
            v_Tokens.push_back(tok);
            _LexError(103, tok);
            return;
        }
        if (ch == '\'') {
            tmpStr += ch;
            i_Col++;
            Position edPos(i_Line, i_Col);
            tok.Set(A_LEX_CHAR, K_CONSTANT_CHAR, tmpStr, stPos, edPos);
            v_Tokens.push_back(tok);
            return;
        }
        else {
            //去掉首位的单引号，只取字符，把两个单引号之间的所有字符都算作错误的字符
            _FillUselessCharacter(105, tmpStr, stPos, ch);
            return;
        }
    }
}

void Scanner::_ScanSpace() {
    string tmpStr;
    Position stPos;
    Position edPos;
    Token tok;
    tmpStr.clear();
    tmpStr = " ";
    stPos.Set(i_Line, i_Col);
    edPos.Set(i_Line, i_Col);
    tok.Set(A_LEX_SPACE, K_SPACE, tmpStr, stPos, edPos);
    v_Tokens.push_back(tok);
}

void Scanner::_FillUselessCharacter(int errorCode, string tmpStr, Position stPos, char ch) {
    Token tok;
    while (ch != '\'') {
        tmpStr += ch;
        i_Col++;
        ch = _ReadChar();
        if (ch == EOF || (ch == 0 && b_EndTag)) {
            b_TrueEndTag = true;
            Position edPos(i_Line, i_Col);
            tok.Set(A_LEX_ERROR, K_CONSTANT_CHAR, tmpStr, stPos, edPos);
            v_Tokens.push_back(tok);
            _LexError(103, tok);
            return;
        }
        if (ch == '\n' || ch == '\t' || ch == '\f' || ch == '\r' || ch == '\b') {
            //转义符，直接报错
            _Untread();
            Position edPos(i_Line, i_Col);
            tok.Set(A_LEX_ERROR, K_CONSTANT_CHAR, tmpStr, stPos, edPos);
            v_Tokens.push_back(tok);
            _LexError(104, tok);
            return;
        }
    }
    //读到了单引号
    tmpStr += ch;
    i_Col++;
    Position edPos(i_Line, i_Col);
    tok.Set(A_LEX_ERROR, K_CONSTANT_CHAR, tmpStr, stPos, edPos);
    v_Tokens.push_back(tok);
    _LexError(errorCode, tok);
    return;
}

void Scanner::_ScanString() {
    //扫描字符串
    char ch = c_CurChar;
    Token tok;
    string tmpStr;
    tmpStr.clear();
    tmpStr = ch;
    Position stPos(i_Line, i_Col);
    ch = _ReadChar();
    if (ch == EOF || (ch == 0 && b_EndTag)) {
        b_TrueEndTag = true;
        Position edPos(i_Line, i_Col);
        tok.Set(A_LEX_ERROR, K_CONSTANT_STRING, tmpStr, stPos, edPos);
        v_Tokens.push_back(tok);
        _LexError(103, tok);
        return;
    }
    if (ch == '\n' || ch == '\t' || ch == '\f' || ch == '\r' || ch == '\b') {
        //转义符，直接报错
        _Untread();
        Position edPos(i_Line, i_Col);
        tok.Set(A_LEX_ERROR, K_CONSTANT_STRING, tmpStr, stPos, edPos);
        v_Tokens.push_back(tok);
        _LexError(104, tok);
        return;
    }
    while (ch != '"') {
        tmpStr += ch;
        i_Col++;
        ch = _ReadChar();
        if (ch == EOF || (ch == 0 && b_EndTag)) {
            b_TrueEndTag = true;
            Position edPos(i_Line, i_Col);
            tok.Set(A_LEX_ERROR, K_CONSTANT_STRING, tmpStr, stPos, edPos);
            v_Tokens.push_back(tok);
            _LexError(103, tok);
            return;
        }
        if (ch == '\n' || ch == '\t' || ch == '\f' || ch == '\r' || ch == '\b') {
            //转义符，直接报错
            _Untread();
            Position edPos(i_Line, i_Col);
            tok.Set(A_LEX_ERROR, K_CONSTANT_STRING, tmpStr, stPos, edPos);
            v_Tokens.push_back(tok);
            _LexError(104, tok);
            return;
        }
    }
    tmpStr += ch;
    i_Col++;
    Position edPos(i_Line, i_Col);
    tok.Set(A_LEX_STRING, K_CONSTANT_STRING, tmpStr, stPos, edPos);
    v_Tokens.push_back(tok);
    return;
}

bool Scanner::_ScanOPR() {
    //扫描运算符
    char ch = c_CurChar;
    Token tok;
    string tmpStr;
    tmpStr.clear();
    tmpStr = ch;
    Position stPos(i_Line, i_Col);
    switch (ch) {
        case '+':
            //对加号，有+23，+，++，+=
            ch = _ReadChar();
            if (ch == EOF || (ch == 0 && b_EndTag)) {
                b_TrueEndTag = true;
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_ERROR, K_OPR, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
                _LexError(103, tok);
                return false;
            }
            //再读一个，看一下是啥
            if (ch == '+') {
                //++
                tmpStr += ch;
                i_Col++;
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_HIGH_PRIORITY, K_OPR_INC, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
            }
            else if (ch == '=') {
                //+=
                tmpStr += ch;
                i_Col++;
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_ASSIGN, K_OPR_ADD_ASSIGN, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
            }
            else if (ch >= '0' && ch <='9') {
                //+23，看一下加号后面有没有合法数字
                c_CurChar = ch;
                i_Col++;
                bool flag = _ScanNumber();
                if (flag) {
                    //后面有合法数字，先把数字拿出来，合成一个正数字在放进去
                    Token tmp = v_Tokens.back();
                    v_Tokens.pop_back();
                    string tmps = tmp.getStr_RealString();
                    tmpStr += tmps;
                    Position edPos = tmp.getPos_EndPos();
                    e_KeywordKind kind = tmp.getEk_kind();
                    if (kind == K_CONSTANT_REAL)
                    {
                        tok.Set(A_LEX_HIGH_PRIORITY, K_CONSTANT_POSITIVE_REAL, tmpStr, stPos, edPos);
                    }
                    else
                    {
                        tok.Set(A_LEX_HIGH_PRIORITY, K_CONSTANT_POSITIVE_INTEGER, tmpStr, stPos, edPos);
                    }
                    v_Tokens.push_back(tok);
                }
                else {
                    //后面的数字不合法，加号视为独立运算符，但是要调整顺序，把放进去的错数字拿出来，先放进去加号，在把数字扔进去
                    Token tmp = v_Tokens.back();
                    v_Tokens.pop_back();
                    Position edPos(i_Line, i_Col - 1);
                    tok.Set(A_LEX_ADD_SUB, K_OPR_ADD, tmpStr, stPos, edPos);
                    v_Tokens.push_back(tok);
                    v_Tokens.push_back(tmp);
                }
            }
            else if (ch == '.') {
                //看看是不是数字
                c_CurChar = ch;
                i_Col++;
                int flag = _ScanBoundary();
                if (flag == 0) {
                    //.只是界符，加号就只能是加号
                    Position edPos(i_Line, i_Col - 1);
                    tok.Set(A_LEX_ADD_SUB, K_OPR_ADD, tmpStr, stPos, edPos);
                    Token tmp = v_Tokens.back();
                    v_Tokens.pop_back();
                    v_Tokens.push_back(tok);
                    v_Tokens.push_back(tmp);
                }
                else if (flag == 1) {
                    //后面是一个数字，变成正实常量
                    Token tmp = v_Tokens.back();
                    v_Tokens.pop_back();
                    string tmps = tmp.getStr_RealString();
                    tmpStr += tmps;
                    Position edPos = tmp.getPos_EndPos();
                    tok.Set(A_LEX_REAL, K_CONSTANT_POSITIVE_REAL, tmpStr, stPos, edPos);
                    v_Tokens.push_back(tok);
                }
                else {
                    //后面什么都不是，加号就只能是加号
                    Position edPos(i_Line, i_Col - 1);
                    tok.Set(A_LEX_ADD_SUB, K_OPR_ADD, tmpStr, stPos, edPos);
                    Token tmp = v_Tokens.back();
                    v_Tokens.pop_back();
                    v_Tokens.push_back(tok);
                    v_Tokens.push_back(tmp);
                }
            }
            else {
                //什么都不对，就直接一个独立的加号
                _Untread();
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_ADD_SUB, K_OPR_ADD, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
            }
            break;
        case '-':
            //对减号，有-23，-，--，-=
            ch = _ReadChar();
            if (ch == EOF || (ch == 0 && b_EndTag)) {
                b_TrueEndTag = true;
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_ERROR, K_OPR, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
                _LexError(103, tok);
                return false;
            }
            //再读一个，看一下是啥
            if (ch == '-') {
                //--
                tmpStr += ch;
                i_Col++;
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_HIGH_PRIORITY, K_OPR_DEC, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
            }
            else if (ch == '=') {
                //-=
                tmpStr += ch;
                i_Col++;
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_ASSIGN, K_OPR_SUB_ASSIGN, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
            }
            else if (ch >= '0' && ch <='9') {
                //+23，看一下加号后面有没有合法数字
                c_CurChar = ch;
                i_Col++;
                bool flag = _ScanNumber();
                if (flag) {
                    //后面有合法数字，先把数字拿出来，合成一个正数字在放进去
                    Token tmp = v_Tokens.back();
                    v_Tokens.pop_back();
                    string tmps = tmp.getStr_RealString();
                    tmpStr += tmps;
                    Position edPos = tmp.getPos_EndPos();
                    e_KeywordKind kind = tmp.getEk_kind();
                    if (kind == K_CONSTANT_REAL)
                    {
                        tok.Set(A_LEX_HIGH_PRIORITY, K_CONSTANT_NEGATIVE_REAL, tmpStr, stPos, edPos);
                    }
                    else
                    {
                        tok.Set(A_LEX_HIGH_PRIORITY, K_CONSTANT_NEGATIVE_INTEGER, tmpStr, stPos, edPos);
                    }
                    v_Tokens.push_back(tok);
                }
                else {
                    //后面的数字不合法，加号视为独立运算符，但是要调整顺序，把放进去的错数字拿出来，先放进去加号，在把数字扔进去
                    Token tmp = v_Tokens.back();
                    v_Tokens.pop_back();
                    Position edPos(i_Line, i_Col - 1);
                    tok.Set(A_LEX_ADD_SUB, K_OPR_SUB, tmpStr, stPos, edPos);
                    v_Tokens.push_back(tok);
                    v_Tokens.push_back(tmp);
                }
            }
            else if (ch == '.') {
                //看看是不是数字
                c_CurChar = ch;
                i_Col++;
                int flag = _ScanBoundary();
                if (flag == 0) {
                    //.只是界符，减号就只能是减号
                    Position edPos(i_Line, i_Col - 1);
                    tok.Set(A_LEX_ADD_SUB, K_OPR_SUB, tmpStr, stPos, edPos);
                    Token tmp = v_Tokens.back();
                    v_Tokens.pop_back();
                    v_Tokens.push_back(tok);
                    v_Tokens.push_back(tmp);
                }
                else if (flag == 1) {
                    //后面是一个数字，变成正实常量
                    Token tmp = v_Tokens.back();
                    v_Tokens.pop_back();
                    string tmps = tmp.getStr_RealString();
                    tmpStr += tmps;
                    Position edPos = tmp.getPos_EndPos();
                    tok.Set(A_LEX_REAL, K_CONSTANT_NEGATIVE_REAL, tmpStr, stPos, edPos);
                    v_Tokens.push_back(tok);
                }
                else {
                    //后面什么都不是，减号就只能是减号
                    Position edPos(i_Line, i_Col - 1);
                    tok.Set(A_LEX_ADD_SUB, K_OPR_SUB, tmpStr, stPos, edPos);
                    Token tmp = v_Tokens.back();
                    v_Tokens.pop_back();
                    v_Tokens.push_back(tok);
                    v_Tokens.push_back(tmp);
                }
            }
            else {
                //什么都不对，就直接一个独立的加号
                _Untread();
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_ADD_SUB, K_OPR_SUB, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
            }
            break;
        case '*':
            //对乘号，有* *=
            ch = _ReadChar();
            if (ch == EOF || (ch == 0 && b_EndTag)) {
                b_TrueEndTag = true;
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_ERROR, K_OPR, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
                _LexError(103, tok);
                return false;
            }
            if (ch == '=') {
                //*=
                tmpStr += ch;
                i_Col++;
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_ASSIGN, K_OPR_MUL_ASSIGN, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
            }
            else {
                //*
                _Untread();
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_MUL_DIV_MOD, K_OPR_MUL, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
            }
        case '/':
            //对除号，有/ /= 和注释
            ch = _ReadChar();
            if (ch == EOF || (ch == 0 && b_EndTag)) {
                b_TrueEndTag = true;
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_ERROR, K_OPR, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
                _LexError(103, tok);
                return false;
            }
            if (ch == '=') {
                // /=
                tmpStr += ch;
                i_Col++;
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_ASSIGN, K_OPR_DIV_ASSIGN, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
            }
            else if (ch  == '/'){
                //是注释
                tmpStr += ch;
                i_Col++;
                _ScanComment(tmpStr, stPos);
            }
            else {
                // /
                _Untread();
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_MUL_DIV_MOD, K_OPR_DIV, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
            }
            break;
        case '%':
            //对取模号，有 % %=
            ch = _ReadChar();
            if (ch == EOF || (ch == 0 && b_EndTag)) {
                b_TrueEndTag = true;
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_ERROR, K_OPR, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
                _LexError(103, tok);
                return false;
            }
            if (ch == '=') {
                //%=
                tmpStr += ch;
                i_Col++;
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_ASSIGN, K_OPR_MOD_ASSIGN, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
            }
            else {
                //%
                _Untread();
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_MUL_DIV_MOD, K_OPR_MOD, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
            }
            break;
        case '=':
            //对等号，有 = ==
            ch = _ReadChar();
            if (ch == EOF || (ch == 0 && b_EndTag)) {
                b_TrueEndTag = true;
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_ERROR, K_OPR, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
                _LexError(103, tok);
                return false;
            }
            if (ch == '=') {
                //==
                tmpStr += ch;
                i_Col++;
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_EQUAL, K_OPR_EQU, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
            }
            else {
                //=
                _Untread();
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_ASSIGN, K_OPR_ASSIGN, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
            }
            break;
        case '&':
            //有 & &= &&
            ch = _ReadChar();
            if (ch == EOF || (ch == 0 && b_EndTag)) {
                b_TrueEndTag = true;
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_ERROR, K_OPR, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
                _LexError(103, tok);
                return false;
            }
            if (ch == '=') {
                //&=
                tmpStr += ch;
                i_Col++;
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_ASSIGN, K_OPR_AND_ASSIGN, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
            }
            else if (ch == '&') {
                //&&
                tmpStr += ch;
                i_Col++;
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_AND, K_OPR_AND, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
            }
            else {
                //&
                _Untread();
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_AND_BIT, K_OPR_AND_BIT, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
            }
            break;
        case '|':
            // | |= ||
            ch = _ReadChar();
            if (ch == EOF || (ch == 0 && b_EndTag)) {
                b_TrueEndTag = true;
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_ERROR, K_OPR, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
                _LexError(103, tok);
                return false;
            }
            if (ch == '=') {
                //|=
                tmpStr += ch;
                i_Col++;
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_ASSIGN, K_OPR_OR_ASSIGN, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
            }
            else if (ch == '|') {
                //||
                tmpStr += ch;
                i_Col++;
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_OR, K_OPR_OR, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
            }
            else {
                //|
                _Untread();
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_OR_BIT, K_OPR_OR_BIT, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
            }
            break;
        case '^':
            //有 ^ ^=
            ch = _ReadChar();
            if (ch == EOF || (ch == 0 && b_EndTag)) {
                b_TrueEndTag = true;
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_ERROR, K_OPR, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
                _LexError(103, tok);
                return false;
            }
            if (ch == '=') {
                //^=
                tmpStr += ch;
                i_Col++;
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_ASSIGN, K_OPR_NOT_ASSIGN, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
            }
            else {
                //^
                _Untread();
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_XOR_BIT, K_OPR_XOR_BIT, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
            }
            break;
        case '~':
            //有 ~
            ch = _ReadChar();
            if (ch == EOF || (ch == 0 && b_EndTag)) {
                b_TrueEndTag = true;
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_ERROR, K_OPR, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
                _LexError(103, tok);
                return false;
            }
            else {
                //~
                _Untread();
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_HIGH_PRIORITY, K_OPR_NOT_BIT, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
            }
            break;
        case '!':
            //有 ! !=
            ch = _ReadChar();
            if (ch == EOF || (ch == 0 && b_EndTag)) {
                b_TrueEndTag = true;
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_ERROR, K_OPR, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
                _LexError(103, tok);
                return false;
            }
            if (ch == '=') {
                // !=
                tmpStr += ch;
                i_Col++;
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_EQUAL, K_OPR_NEQU, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
            }
            else {
                // !
                _Untread();
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_HIGH_PRIORITY, K_OPR_NOT, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
            }
            break;
        case '?':
            //有 ?
            ch = _ReadChar();
            if (ch == EOF || (ch == 0 && b_EndTag)) {
                b_TrueEndTag = true;
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_ERROR, K_OPR, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
                _LexError(103, tok);
                return false;
            }
            else {
                //?
                _Untread();
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_TERNARY, K_OPR_TERNARY_QUESTION, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
            }
            break;
        case ':':
            //有 :
            ch = _ReadChar();
            if (ch == EOF || (ch == 0 && b_EndTag)) {
                b_TrueEndTag = true;
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_ERROR, K_OPR, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
                _LexError(103, tok);
                return false;
            }
            else {
                //:
                _Untread();
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_TERNARY, K_OPR_TERNARY_COLON, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
            }
            break;
        case '<':
            // 有 < << <= <<=
            ch = _ReadChar();
            if (ch == EOF || (ch == 0 && b_EndTag)) {
                b_TrueEndTag = true;
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_ERROR, K_OPR, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
                _LexError(103, tok);
                return false;
            }
            if (ch == '=') {
                //<=
                tmpStr += ch;
                i_Col++;
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_COMPARE, K_OPR_LE, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
            }
            else if (ch == '<') {
                //<< <<=
                tmpStr += ch;
                i_Col++;
                ch = _ReadChar();
                if (ch == EOF || (ch == 0 && b_EndTag)) {
                    b_TrueEndTag = true;
                    Position edPos(i_Line, i_Col);
                    tok.Set(A_LEX_ERROR, K_OPR, tmpStr, stPos, edPos);
                    v_Tokens.push_back(tok);
                    _LexError(103, tok);
                    return false;
                }
                if (ch == '=') {
                    //<<=
                    tmpStr += ch;
                    i_Col++;
                    Position edPos(i_Line, i_Col);
                    tok.Set(A_LEX_ASSIGN, K_OPR_SHIFT_LEFT_ASSIGN, tmpStr, stPos, edPos);
                    v_Tokens.push_back(tok);
                }
                else {
                    //<<
                    _Untread();
                    Position edPos(i_Line, i_Col);
                    tok.Set(A_LEX_SHIFT, K_OPR_SHIFT_LEFT, tmpStr, stPos, edPos);
                    v_Tokens.push_back(tok);
                }

            }
            else {
                //<
                _Untread();
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_COMPARE, K_OPR_LT, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
            }
            break;
        case '>':
            // 有 > >> >>> >= >>= >>>=
            ch = _ReadChar();
            if (ch == EOF || (ch == 0 && b_EndTag)) {
                b_TrueEndTag = true;
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_ERROR, K_OPR, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
                _LexError(103, tok);
                return false;
            }
            if (ch == '=') {
                //>=
                tmpStr += ch;
                i_Col++;
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_COMPARE, K_OPR_GE, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
            }
            else if (ch == '>') {
                //>> >>> >>= >>>=
                tmpStr += ch;
                i_Col++;
                ch = _ReadChar();
                if (ch == EOF || (ch == 0 && b_EndTag)) {
                    b_TrueEndTag = true;
                    Position edPos(i_Line, i_Col);
                    tok.Set(A_LEX_ERROR, K_OPR, tmpStr, stPos, edPos);
                    v_Tokens.push_back(tok);
                    _LexError(103, tok);
                    return false;
                }
                if (ch == '=') {
                    //>>=
                    tmpStr += ch;
                    i_Col++;
                    Position edPos(i_Line, i_Col);
                    tok.Set(A_LEX_ASSIGN, K_OPR_SHIFT_RIGHT_ASSIGN, tmpStr, stPos, edPos);
                    v_Tokens.push_back(tok);
                }
                else if (ch == '>') {
                    //>>> >>>=
                    tmpStr += ch;
                    i_Col++;
                    ch = _ReadChar();
                    if (ch == EOF || (ch == 0 && b_EndTag)) {
                        b_TrueEndTag = true;
                        Position edPos(i_Line, i_Col);
                        tok.Set(A_LEX_ERROR, K_OPR, tmpStr, stPos, edPos);
                        v_Tokens.push_back(tok);
                        _LexError(103, tok);
                        return false;
                    }
                    if (ch == '=') {
                        //>>>=
                        tmpStr += ch;
                        i_Col++;
                        Position edPos(i_Line, i_Col);
                        tok.Set(A_LEX_ASSIGN, K_OPR_SHIFT_ZERO_ASSIGN, tmpStr, stPos, edPos);
                        v_Tokens.push_back(tok);
                    }
                    else {
                        //>>>
                        _Untread();
                        Position edPos(i_Line, i_Col);
                        tok.Set(A_LEX_SHIFT, K_OPR_SHIFT_ZERO, tmpStr, stPos, edPos);
                        v_Tokens.push_back(tok);
                    }
                }
                else {
                    //>>
                    _Untread();
                    Position edPos(i_Line, i_Col);
                    tok.Set(A_LEX_SHIFT, K_OPR_SHIFT_RIGHT, tmpStr, stPos, edPos);
                    v_Tokens.push_back(tok);
                }
            }
            else {
                //>
                _Untread();
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_COMPARE, K_OPR_GT, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
            }
            break;
        default:break;
    }
    return true;
}

int Scanner::_ScanBoundary() {
    //扫描界符
    char ch = c_CurChar;
    Token tok;
    string tmpStr;
    tmpStr.clear();
    tmpStr = ch;
    Position stPos(i_Line, i_Col);
    Position edPos;
    switch (ch) {
        case '{':
            edPos.Set(i_Line, i_Col);
            tok.Set(A_LEX_BRACES, K_BOUNDARY_BRACES_LEFT, tmpStr, stPos, edPos);
            v_Tokens.push_back(tok);
            break;
        case '}':
            edPos.Set(i_Line, i_Col);
            tok.Set(A_LEX_BRACES, K_BOUNDARY_BRACES_RIGHT, tmpStr, stPos, edPos);
            v_Tokens.push_back(tok);
            break;
        case '[':
            edPos.Set(i_Line, i_Col);
            tok.Set(A_LEX_BOUNDARY, K_BOUNDARY_BRACKET_LEFT, tmpStr, stPos, edPos);
            v_Tokens.push_back(tok);
            break;
        case ']':
            edPos.Set(i_Line, i_Col);
            tok.Set(A_LEX_BOUNDARY, K_BOUNDARY_BRACKET_RIGHT, tmpStr, stPos, edPos);
            v_Tokens.push_back(tok);
            break;
        case '(':
            edPos.Set(i_Line, i_Col);
            tok.Set(A_LEX_BOUNDARY, K_BOUNDARY_PARENTHESES_LEFT, tmpStr, stPos, edPos);
            v_Tokens.push_back(tok);
            break;
        case ')':
            edPos.Set(i_Line, i_Col);
            tok.Set(A_LEX_BOUNDARY, K_BOUNDARY_PARENTHESES_RIGHT, tmpStr, stPos, edPos);
            v_Tokens.push_back(tok);
            break;
        case ',':
            edPos.Set(i_Line, i_Col);
            tok.Set(A_LEX_COMMA, K_BOUNDARY_COMMA, tmpStr, stPos, edPos);
            v_Tokens.push_back(tok);
            break;
        case ';':
            edPos.Set(i_Line, i_Col);
            tok.Set(A_LEX_SEMICOLON, K_BOUNDARY_SEMICOLON, tmpStr, stPos, edPos);
            v_Tokens.push_back(tok);
            break;
        case '.':
            ch = _ReadChar();
            if (ch == EOF || (ch == 0 && b_EndTag)) {
                b_TrueEndTag = true;
                edPos.Set(i_Line, i_Col);
                tok.Set(A_LEX_ERROR, K_BOUNDARY, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
                _LexError(103, tok);
                return -1;
            }
            if (ch >= '0' && ch < '9') {
                //可能是小数点
                while (ch >= '0' && ch < '9') {
                    //.123
                    tmpStr += ch;
                    i_Col++;
                    ch = _ReadChar();
                    if (ch == EOF || (ch == 0 && b_EndTag)) {
                        b_TrueEndTag = true;
                        edPos.Set(i_Line, i_Col);
                        tok.Set(A_LEX_ERROR, K_BOUNDARY, tmpStr, stPos, edPos);
                        v_Tokens.push_back(tok);
                        _LexError(103, tok);
                        return -1;
                    }
                }
                if (ch == 'E' || ch == 'e') {
                    //.123E，继续读
                    tmpStr += ch;
                    i_Col++;
                    ch = _ReadChar();
                    if (ch == EOF || (ch == 0 && b_EndTag)) {
                        b_TrueEndTag = true;
                        edPos.Set(i_Line, i_Col);
                        tok.Set(A_LEX_ERROR, K_BOUNDARY, tmpStr, stPos, edPos);
                        v_Tokens.push_back(tok);
                        _LexError(103, tok);
                        return -1;
                    }
                    if (ch == '-') {
                        //E后面很可能跟了一个负号
                        tmpStr += ch;
                        i_Col++;
                        ch = _ReadChar();
                        if (ch == EOF || (ch == 0 && b_EndTag)) {
                            b_TrueEndTag = true;
                            Position edPos(i_Line, i_Col);
                            tok.Set(A_LEX_ERROR, K_CONSTANT_REAL, tmpStr, stPos, edPos);
                            v_Tokens.push_back(tok);
                            _LexError(103, tok);
                            return false;
                        }
                    }
                    if (ch >= '0' && ch <= '9') {
                        //.123E2
                        while (ch >= '0' && ch < '9') {
                            tmpStr += ch;
                            i_Col++;
                            ch = _ReadChar();
                            if (ch == EOF || (ch == 0 && b_EndTag)) {
                                b_TrueEndTag = true;
                                edPos.Set(i_Line, i_Col);
                                tok.Set(A_LEX_ERROR, K_BOUNDARY, tmpStr, stPos, edPos);
                                v_Tokens.push_back(tok);
                                _LexError(103, tok);
                                return -1;
                            }
                        }
                        if (ch == 'F' || ch == 'f' || ch == 'D' || ch == 'd') {
                            //.123E3F
                            tmpStr += ch;
                            i_Col++;
                            edPos.Set(i_Line, i_Col);
                            tok.Set(A_LEX_REAL, K_CONSTANT_REAL, tmpStr, stPos, edPos);
                            v_Tokens.push_back(tok);
                            return 1;
                        }
                        else {
                            //.123E2
                            _Untread();
                            edPos.Set(i_Line, i_Col);
                            tok.Set(A_LEX_REAL, K_CONSTANT_REAL, tmpStr, stPos, edPos);
                            v_Tokens.push_back(tok);
                            return 1;
                        }
                    }
                    else {
                        //.123E没有数字，出错
                        _Untread();
                        edPos.Set(i_Line, i_Col);
                        tok.Set(A_LEX_ERROR, K_CONSTANT_REAL, tmpStr, stPos, edPos);
                        v_Tokens.push_back(tok);
                        _LexError(100, tok);
                        return -1;
                    }
                }
                else if (ch == 'F' || ch == 'f' || ch == 'D' || ch == 'd') {
                    //.123F
                    tmpStr += ch;
                    i_Col++;
                    edPos.Set(i_Line, i_Col);
                    tok.Set(A_LEX_REAL, K_CONSTANT_REAL, tmpStr, stPos, edPos);
                    v_Tokens.push_back(tok);
                    return 1;
                }
                else {
                    //.123
                    _Untread();
                    edPos.Set(i_Line, i_Col);
                    tok.Set(A_LEX_REAL, K_CONSTANT_REAL, tmpStr, stPos, edPos);
                    v_Tokens.push_back(tok);
                    return 1;
                }
            }
            else {
                //只是界符
                _Untread();
                edPos.Set(i_Line, i_Col);
                tok.Set(A_LEX_BOUNDARY, K_BOUNDARY_PERIOD, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
            }
            break;
        default:break;
    }
    return 0;
}

void Scanner::_ScanComment(string tmpStr, Position stPos) {
    Token tok;
    //扫描注释
    char ch = _ReadChar();
    if (ch == EOF || (ch == 0 && b_EndTag)) {
        b_TrueEndTag = true;
        Position edPos(i_Line, i_Col);
        tok.Set(A_LEX_ERROR, K_CONSTANT_STRING, tmpStr, stPos, edPos);
        v_Tokens.push_back(tok);
        _LexError(103, tok);
        return;
    }
    if (ch == '\n' || ch == '\t' || ch == '\f' || ch == '\r' || ch == '\b') {
        //转义符，终止
        _Untread();
        Position edPos(i_Line, i_Col);
        tok.Set(A_LEX_COMMENT, K_COMMENT, tmpStr, stPos, edPos);
        v_Tokens.push_back(tok);
        return;
    }
    while (!(ch == '\n' || ch == '\t' || ch == '\f' || ch == '\r' || ch == '\b')) {
        tmpStr += ch;
        i_Col++;
        ch = _ReadChar();
        if (ch == '\n' || ch == '\t' || ch == '\f' || ch == '\r' || ch == '\b') {
            //转义符，终止
            _Untread();
            Position edPos(i_Line, i_Col);
            tok.Set(A_LEX_COMMENT, K_COMMENT, tmpStr, stPos, edPos);
            v_Tokens.push_back(tok);
            return;
        }
    }
}

void Scanner::_ErrOutput() {
    int len = v_Err_Tokens.size();
    for (int i = 0; i < len; ++i) {
        Token tmp = v_Err_Tokens[i];
        string err = v_Err_String[i];
        Position pos = tmp.getPos_StartPos();
        of_ErrFile << "Error: " << err << endl;
        cerr << "Error: " << err << endl;
        of_ErrFile << "\tAt Line " << pos.getI_Line() << ", Column " << pos.getI_Col() << endl;
        cerr << "\tAt Line " << pos.getI_Line() << ", Column " << pos.getI_Col() << endl;
    }
}
