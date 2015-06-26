#include <OnvifOperation.h>
#include <iostream>
using namespace std;

int main(int argc, char* argv[])
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

