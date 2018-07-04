//
// Created by Gary on 2018.7.3.
// Scanner类，用于进行词法分析的主类
//

#ifndef SCANNER_SCANNER_H
#define SCANNER_SCANNER_H

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include "Token.h"
#include "Position.h"
#include "Constants.h"
using namespace std;

class Scanner {
public:
    Scanner();
    ~Scanner();

    /**
     * 初始化
     * @param fileName 输入文件名
     */
    void Init(string fileName);
    /**
     * 执行
     */
    void Execute();

private:
    ifstream if_InFile;//输入文件
    ofstream of_OutFile, of_ErrFile;//输出结果文件和错误结果文件
    string str_InFileName;//输入文件名
    string str_OutFileName;//输出文件名
    string str_ErrFileName;//错误文件名
    string str_InFilePathName;//输入文件路径

    string str_Buffer, str_BufferL, str_BufferR;//缓冲区，分别可存64，32和32个字符，后两者覆盖前面的缓冲区的左右两部分
    int i_ForwardCount;//forward指针，最大为31（0-31）
    bool b_IsRightBufferAvail;//判断右缓冲是否有东西
    int line, col;//当前读出字符的行号和列号
    /**
     * 用于从缓冲区读取一个字符
     */
    char _ReadChar();
    /**
     * 用于将文件内容读入第二缓冲区
     */
    void _LoadToLo();
    /**
     * 用于将第二缓冲区内容转入第一缓冲区（第一缓冲区内容转入L）
     */
    void _LoToHi();
    /**
     * 用于将第二缓冲区转移到R缓冲区
     */
    void _LoToR();
    /**
     * 回退一个字符
     */
    void _Untread();

    /**
     * 词法分析主控
     */
    void _Scan();
};


#endif //SCANNER_SCANNER_H
