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
        of_OutFile.close();
        of_ErrFile.close();
        cout << endl << "分析完成" << endl;
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
    string tmpStr;
    Position stPos;
    Position edPos;
    Token tok;
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
            tmpStr = " ";
            stPos.Set(i_Line, i_Col);
            edPos.Set(i_Line, i_Col);
            tok.Set(A_LEX_SPACE, K_SPACE, tmpStr, stPos, edPos);
            v_Tokens.push_back(tok);
            break;
        case '\'':
            i_Col++;
            _ScanCharacter();
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
        cout << tmp.getStr_RealString() << " (" << tmp.getPos_StartPos().getI_Line() << ", "
             << tmp.getPos_StartPos().getI_Col() << ") (" << tmp.getPos_EndPos().getI_Line() << ", "
             << tmp.getPos_EndPos().getI_Col() << ")" << endl;
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
                    if (!(ch >= '0' && ch <= '9')) {
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
            if (!((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'F') || (ch >= 'a' && ch <= 'f')))
            {
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
            else
            {
                while ((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'F') || (ch >= 'a' && ch <= 'f'))
                {
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
                if (ch == 'L' || ch == 'l')
                {
                    //0x123L
                    tmpStr += ch;
                    i_Col++;
                }
                else
                {
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
            while (ch >= '0' && ch <= '9')
            {
                if (octFlag)
                {
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
            if (ch == 'F' || ch == 'f' | ch == 'D' || ch == 'd')
            {
                //这个时候认为是十进制实型
                tmpStr += ch;
                i_Col++;
                Position edPos(i_Line, i_Col);
                tok.Set(A_LEX_REAL, K_CONSTANT_REAL, tmpStr, stPos, edPos);
                v_Tokens.push_back(tok);
                return true;
            }
            else
            {
                //这个时候认为是八进制
                _Untread();
                Position edPos(i_Line, i_Col);
                if (octFlag)
                {
                    tok.Set(A_LEX_INT, K_CONSTANT_OCT_INTEGER, tmpStr, stPos, edPos);
                    v_Tokens.push_back(tok);
                    return true;
                }
                else
                {
                    //不再符合八进制
                    tok.Set(A_LEX_ERROR, K_CONSTANT_OCT_INTEGER, tmpStr, stPos, edPos);
                    v_Tokens.push_back(tok);
                    _LexError(102, tok);
                    return false;
                }
            }
        }
        else
        {
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
                //1.32E3继续读
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
            //123E3继续读
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
    switch (errorCode) {
        case 100:
            cerr << "实型常量格式错误" << endl;
            break;
        case 101:
            cerr << "十六进制数格式错误" << endl;
            break;
        case 102:
            cerr << "八进制数格式错误" << endl;
            break;
        case 103 :
            cerr << "程序结尾非法" << endl;
            break;
        case 104:
            cerr << "程序行尾非法" << endl;
            break;
        default:
            break;
    }
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
    if (ch == '\n' || ch == '\t' || ch == '\f' || ch == '\r' || ch == '\b')
    {
        //转义符，直接报错
        _Untread();
        Position edPos(i_Line, i_Col);
        tok.Set(A_LEX_ERROR, K_ESCAPE_SINGLE_QUO, tmpStr, stPos, edPos);
        v_Tokens.push_back(tok);
        _LexError(104, tok);
        return;
    }
    else if (ch == '\\')
    {
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
        char tmp = ch;
        //再读一个字符看看是否正常结束
                //TODO::
    }
    else
    {
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
        if (ch == '\'')
        {
            //就不往tmp里面放了，只是取那个字符
            i_Col++;
            Position edPos(i_Line, i_Col);
            char ans = tmpStr.at(1);
            tmpStr = ans;
            tok.Set(A_LEX_CHAR, K_CONSTANT_CHAR, tmpStr, stPos, edPos);
            v_Tokens.push_back(tok);
            return;
        }
        else
        {
            //去掉首位的单引号，只取字符
            char ans1 = tmpStr.at(1);
            char ans2 = tmpStr.at(2);
            tmpStr = ans1;
            tmpStr += ans2;

        }
    }
}
