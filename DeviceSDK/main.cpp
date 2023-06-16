// DeviceSDK.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include "Core.h"
Core mCore;
int main()
{
    mCore.testHk();
    std::cout << "Hello World!\n";
    while (true) {
        Sleep(1000);
    }
}
