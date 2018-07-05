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
        _LoadToLo();
        _LoToHi();
        _LoadToLo();
        while (true) {
            _Scan();
            if (b_TrueEndTag)
            {
                break;
            }
        }
        of_OutFile.close();
        of_ErrFile.close();
        cout << endl << "分析完成" << endl;
    }
    else
    {
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
    if (i_ForwardCount >= (CON_BUFFER_SIZE / 2))
    {
        //读完一个缓冲区，需要加载下一个缓冲区
        if (b_IsRightBufferAvail)
        {
            //第二个缓冲区里面有东西
            _LoToHi();
            _LoToR();
        }
        else if (!b_EOFTag)
        {
            _LoToHi();
            _LoadToLo();
        }
        else if (!b_EndTag)
        {
            _LoToHi();
            b_EndTag = true;
        }
        else
        {
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
    if (if_InFile.eof())
    {
        b_EOFTag = true;
    }
    for (int i = CON_BUFFER_SIZE / 2; i < CON_BUFFER_SIZE; ++i)
    {
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
    if (i_ForwardCount < 0)
    {
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
    if (ch == EOF || (ch == 0 && b_EndTag))
    {
        b_TrueEndTag = true;
        return;
    }
    c_CurChar = ch;
    switch (c_CurChar)
    {
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
        default:
            break;
    }
    cout << ch;
}

void Scanner::_ScanIdentityOrKeywordOrBoolean() {
    char ch = c_CurChar;
    Token tok;
    string tmpStr;//用于临时存放单词
    tmpStr.clear();
    tmpStr = ch;
    ch = _ReadChar();//继续往后读
    Position stPos;
    stPos.Set(i_Line, i_Col);
    while ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || (ch == '$') || (ch == '_') || (ch >= '0' && ch <='9'))
    {
        tmpStr += ch;
        i_Col++;
        ch = _ReadChar();
    }
    //最后一个字符不符合要求，回退之
    _Untread();
    i_Col--;
    //布尔常量
    if (_IsBoolean(tmpStr) != 0)
    {

    }

}

int Scanner::_IsBoolean(string str) {
    if (str == "true")
    {
        return 1;
    }
    else if (str == "false")
    {
        return 2;
    }
    return 0;
}

int Scanner::_IsKeyword(string str) {
    //50个关键字直接查表
    for (int i = 0; i < 50; ++i) {
        string tmp(ca_Keyword[i]);
        if (str == tmp)
        {
            return i;
        }
    }
    return -1;
}
