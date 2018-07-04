//
// Created by Gary on 2018.7.4.
//

#include "Token.h"

void Token::Set(e_Attributes ea, e_KeywordKind ek, string str, Position p) {
    ea_Type = ea;
    ek_kind = ek;
    str_RealString = str;
    pos_CodePos = p;
}
