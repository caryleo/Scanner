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

class Token;

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
    int i_Line, i_Col;//当前读出字符的行号和列号
    bool b_EOFTag;//读文件结束标志
    bool b_EndTag;//读字符结束标志
    bool b_TrueEndTag;//分析结束标志
    char c_CurChar;//当前读出字符

    vector<Token> v_Tokens;
    vector<Token> v_Err_Tokens;
    vector<string> v_Err_String;
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
    /**
     * 标识符，关键字，布尔常量分析
     */
    void _ScanIdentityOrKeywordOrBoolean();
    /**
     * 判断当前字段是否为布尔常量
     * @param str 欲检测的字段
     * @return 0则不是布尔常量，1则为true，2则为false
     */
    int _IsBoolean(string str);
    /**
     * 判断当前字段是否为关键字
     * @param str 欲检测的字段
     * @return 保留字数组的下标，-1则不存在
     */
    int _IsKeyword(string str);
    /**
     * 数字常量分析
     * @return true是分析正确，false是分析错误
     */
    bool _ScanNumber();
    /**
     * 用于进行文件输出
     */
    void _FileOutput();
    /**
     * 词法分析报错
     * @param errorCode 错误码
     * @param tok 错误的token
     */
    void _LexError(int errorCode, Token tok);
    /**
     * 扫描字符
     */
    void _ScanCharacter();
    /**
     * 扫描空格
     */
    void _ScanSpace();
    /**
     * 对于已经无效的字符，进行后续内容的补全
     * @param errorCode 需要传入的错误码
     * @param tmpStr 没有补全的内容
     * @param stPos 开始位置
     * @param ch 当前字符
     */
    void _FillUselessCharacter(int errorCode, string tmpStr, Position stPos, char ch);
    /**
     * 扫描字符串
     */
    void _ScanString();
    /**
     * 扫描运算符
     * @return true成功，false失败
     */
    bool _ScanOPR();
    /**
     * 扫描界符
     * @return -1错误，0正确，1识别出数字
     */
    int _ScanBoundary();
    /**
     * 扫描注释
     * @param tmpStr 之前的字符串
     * @param stPos  开始位置
     */
    void _ScanComment(string tmpStr, Position stPos);
    /**
     * 输出词法错误
     */
    void _ErrOutput();
};


#endif //SCANNER_SCANNER_H
