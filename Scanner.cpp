//
// Created by Gary on 2018.7.3.
//

#include <cstring>
#include "Scanner.h"

Scanner::~Scanner() = default;

Scanner::Scanner() = default;

void Scanner::Execute() {
    _FileProcess();
}

void Scanner::Init(string fileName) {
    //对文件名进行加工
    str_InFileName = fileName;
    str_InFilePathName = "..\\Data\\" + str_InFileName + ".java";
    str_OutFileName = "..\\Data\\" + str_InFileName + "_Out.txt";
    str_ErrFileName = "..\\Data\\" + str_InFileName + "_Err.txt";
}

void Scanner::_FileProcess() {
    if_InFile.open(str_InFilePathName, ios::in);
    if (if_InFile.good())
    {

        of_OutFile.open(str_OutFileName, ios::out);
        of_ErrFile.open(str_ErrFileName, ios::out);
        int lineCount = 1;
        while (!if_InFile.eof())
        {
            string str;
            getline(if_InFile, str);
            cout << "LINE" << lineCount++ << ":" << str << endl;
        }
        of_OutFile.close();
        of_ErrFile.close();
    }
    else
    {
        cerr << "Open InFile Error!" << endl;
    }
    if_InFile.close();
}
