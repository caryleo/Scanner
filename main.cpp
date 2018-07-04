#include <iostream>
#include "Constants.h"
#include "Scanner.h"

int main() {
    cout << "请输入要进行分析的文件: ";
    string fileName;
    cin >> fileName;
    Scanner scanner;
    scanner.Init(fileName);
    scanner.Execute();
    return 0;
}