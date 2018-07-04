//
// Created by Gary on 2018.7.4.
// 代码位置类
//

#ifndef SCANNER_POSITION_H
#define SCANNER_POSITION_H


class Position {
public:
    int i_Line;
    int i_Col;
    /**
     * 设置位置
     * @param x 行号
     * @param y 列号
     */
    void Set(int x, int y);
};


#endif //SCANNER_POSITION_H
