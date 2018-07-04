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

    string str_buffer, str_bufferL, str_bufferR;//缓冲区，分别可存64，32和32个字符，后两者覆盖前面的缓冲区的左右两部分
    /**
     * 进行读文件处理和写文件处理
     */
    void _FileProcess();
    /**
     * 用于从缓冲区读取一个字符
     */
    char _ReadChar();
};


#endif //SCANNER_SCANNER_H
