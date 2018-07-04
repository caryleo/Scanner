//
// Created by Gary on 2018.7.4.
// 属性字类
//

#ifndef SCANNER_TOKEN_H
#define SCANNER_TOKEN_H
#include "Scanner.h"
#include "Constants.h"
#include "Position.h"
#include <string>

using namespace std;

class Token {
public:
    e_Attributes ea_Type;
    e_KeywordKind ek_kind;
    string str_RealString;
    Position pos_StartPos;
    Position pos_EndPos;
    /**
     * 设置属性字属性
     * @param ea type类型
     * @param ek kind具体类型
     * @param str 对于常量和表示符的具体内容，否则就是保留字项
     * @param stp 词素的位置
     */
    void Set(e_Attributes ea, e_KeywordKind ek, string str, Position stp, Position edp);
};


#endif //SCANNER_TOKEN_H
