// test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <OnvifOperation.h>
#pragma comment(lib,"OnvifOperation.lib")
#include <iostream>
using namespace std;
int _tmain(int argc, _TCHAR* argv[])
{
    if(-1 == init_DLL())
    {
        cout << "initDll failed\n";
        system("pause");
        return -1;
    }

    cout << "\nsearch device\n\n";
    system("pause");

    if(-1 == search_ONVIF_IPC(1))
    {
        cout << "search device failed\n";
        system("pause");
        return -1;
    }

    int num = get_number_of_IPCs();
    if(-1 == num)
    {
        cout << "get_number_of_IPCs failed\n";
        system("pause");
        return -1;
    }

    IPC_URI* IPC_URI_array = new IPC_URI[num];

    if(-1 == get_all_IPC_URIs(IPC_URI_array, num))
    {
        cout << "get_all_IPC_URIs failed\n";
        system("pause");
        return -1;
    }

    for(int i = 0; i < num; i++)
    {
        cout << IPC_URI_array[i].ip << ":" << IPC_URI_array[i].URI << endl;
    }

    cout << "-------------------------------------\n";

    char* URI = new char[256];

    cout << "\nGet URI by IP\n\n";
    system("pause");

    if(-1 == get_IPC_URI_according_to_IP("192.168.10.185", sizeof("192.168.10.185"), URI, 256, "admin", "12345"))
    {
        cout << "192.168.10.185 get_IPC_URI_according_to_IP failed\n";
    }
    else
    {
        cout << "192.168.10.185:" << URI << endl;
    }

    if(-1 == get_IPC_URI_according_to_IP("192.168.10.142", sizeof("192.168.10.142"), URI, 256, "admin", "12345"))
    {
        cout << "192.168.10.142 get_IPC_URI_according_to_IP failed\n";
    }
    else
    {
        cout << "192.168.10.142:" << URI << endl;
    }

    if(-1 == get_IPC_URI_according_to_IP("192.168.10.147", sizeof("192.168.10.147"), URI, 256, "admin", "12345"))
    {
        cout << "192.168.10.147 get_IPC_URI_according_to_IP failed\n";
    }
    else
    {
        cout << "192.168.10.147:" << URI << endl;
    }

    if(-1 == get_IPC_URI_according_to_IP("192.168.10.195", sizeof("192.168.10.195"), URI, 256, "admin", "12345"))
    {
        cout << "192.168.10.195 get_IPC_URI_according_to_IP failed\n";
    }
    else
    {
        cout << "192.168.10.195:" << URI << endl;
    }

    if(-1 == get_IPC_URI_according_to_IP("192.168.10.141", sizeof("192.168.10.141"), URI, 256, "admin", "12345"))
    {
        cout << "192.168.10.141 get_IPC_URI_according_to_IP failed\n";
    }
    else
    {
        cout << "192.168.10.141:" << URI << endl;
    }

    cout << "-------------------------------------\n";

    if(-1 == get_all_IPC_URIs(IPC_URI_array, num))
    {
        cout << "get_all_IPC_URIs failed\n";
        system("pause");
        return -1;
    }

    for(int i = 0; i < num; i++)
    {
        cout << IPC_URI_array[i].ip << ":" << IPC_URI_array[i].URI << endl;
    }

    cout << "-------------------------------------\n";

    delete[] IPC_URI_array;

    cout << "\nSearch again\n\n";
    system("pause");

    if(-1 == search_ONVIF_IPC(1))
    {
        cout << "search device failed\n";
        system("pause");
        return -1;
    }

    num = get_number_of_IPCs();

    if(-1 == num)
    {
        cout << "get_number_of_IPCs failed\n";
        system("pause");
        return -1;
    }

    IPC_URI_array = new IPC_URI[num];

    cout << "getNumOfOnvifDev: " << num << endl;

    if(-1 == get_all_IPC_URIs(IPC_URI_array, num))
    {
        cout << "get_all_IPC_URIs failed\n";
        system("pause");
        return -1;
    }

    for(int i = 0; i < num; i++)
    {
        cout << IPC_URI_array[i].ip << ":" << IPC_URI_array[i].URI << endl;
    }

    cout << "-------------------------------------\n";
    cout << "\nresetDll\n\n";
    system("pause");

    if(-1 == reset_DLL())
    {
        cout << "reset_DLL failed\n";
        system("pause");
        return -1;
    }

    cout << "getNumOfOnvifDev: " << get_number_of_IPCs() << endl;

    delete[] IPC_URI_array;

    delete URI;

    if(-1 == uninit_DLL())
    {
        cout << "uninit_DLL failed\n";
        system("pause");
        return -1;
    }
    system("pause");
    return 0;
}

