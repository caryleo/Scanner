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
        while (!if_InFile.eof()) {

            _Scan();
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
    i_ForwardCount = 0;
    b_IsRightBufferAvail = false;
}

char Scanner::_ReadChar() {
    char ret;
    if (i_ForwardCount >= (CON_BUFFER_SIZE / 2 - 1))
    {
        //读完一个缓冲区，需要加载下一个缓冲区
        if (b_IsRightBufferAvail)
        {
            //第二个缓冲区里面有东西
            _LoToHi();
            _LoToR();
        }
        else
        {
            _LoToHi();
            _LoadToLo();
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

}
