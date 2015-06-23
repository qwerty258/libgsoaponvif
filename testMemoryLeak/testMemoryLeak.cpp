// testMemoryLeak.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <OnvifOperation.h>
#pragma comment(lib,"OnvifOperation.lib")
#include <iostream>
using namespace std;
int _tmain(int argc, _TCHAR* argv[])
{

    cout << "begin to test\n";

    system("pause");

    for(size_t i = 0; i < 10000; ++i)
    {
        init_DLL();
        uninit_DLL();
    }

    cout << "test end\n";

    system("pause");

    return 0;
}

