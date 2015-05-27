// testMemoryLeak.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <OnvifOperation.h>
#pragma comment(lib,"OnvifOperation.lib")
#include <iostream>
using namespace std;
int _tmain(int argc, _TCHAR* argv[])
{
    init_DLL();

    cout << "begin to test\n";

    system("pause");

    for(size_t i = 0; i < 100; ++i)
    {
        reset_DLL();
    }

    cout << "test end\n";

    system("pause");

    uninit_DLL();

    system("pause");

    return 0;
}

