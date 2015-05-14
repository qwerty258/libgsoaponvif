// test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <OnvifOperation.h>
#pragma comment(lib,"OnvifOperation.lib")
#include <iostream>
using namespace std;
int _tmain(int argc, _TCHAR* argv[])
{
    cout << "\nsearch device\n\n";
    system("pause");

    if(-1 == initDll())
    {
        cout << "initDll failed\n";
        system("pause");
        return -1;
    }

    if(-1 == searchDev())
    {
        cout << "search device failed\n";
        system("pause");
        return -1;
    }

    int num = getNumOfOnvifDev();
    if(-1 == num)
    {
        cout << "getNumOfOnvifDev failed\n";
        system("pause");
        return -1;
    }

    deviceInfoArray* infoArray = new deviceInfoArray[num];

    if(-1 == getAllDevURI(infoArray, num))
    {
        cout << "getAllDevURI failed\n";
        system("pause");
        return -1;
    }

    for(int i = 0; i < num; i++)
    {
        cout << infoArray[i].ip << ":" << infoArray[i].URI << endl;
    }

    cout << "-------------------------------------\n";

    char* URI = new char[256];

    cout << "\nGet URI by IP\n\n";
    system("pause");

    if(-1 == getURIFromIP("192.168.10.185", sizeof("192.168.10.185"), URI, 256, "admin", "12345"))
    {
        cout << "192.168.10.185 getURIFromIP failed\n";
    }
    else
    {
        cout << "192.168.10.185:" << URI << endl;
    }

    if(-1 == getURIFromIP("192.168.10.142", sizeof("192.168.10.142"), URI, 256, "admin", "12345"))
    {
        cout << "192.168.10.142 getURIFromIP failed\n";
    }
    else
    {
        cout << "192.168.10.142:" << URI << endl;
    }

    if(-1 == getURIFromIP("192.168.10.147", sizeof("192.168.10.147"), URI, 256, "admin", "12345"))
    {
        cout << "192.168.10.147 getURIFromIP failed\n";
    }
    else
    {
        cout << "192.168.10.147:" << URI << endl;
    }

    if(-1 == getURIFromIP("192.168.10.195", sizeof("192.168.10.195"), URI, 256, "admin", "12345"))
    {
        cout << "192.168.10.195 getURIFromIP failed\n";
    }
    else
    {
        cout << "192.168.10.195:" << URI << endl;
    }

    if(-1 == getURIFromIP("192.168.10.141", sizeof("192.168.10.141"), URI, 256, "admin", "12345"))
    {
        cout << "192.168.10.141 getURIFromIP failed\n";
    }
    else
    {
        cout << "192.168.10.141:" << URI << endl;
    }

    cout << "-------------------------------------\n";

    if(-1 == getAllDevURI(infoArray, num))
    {
        cout << "getAllDevURI failed\n";
        system("pause");
        return -1;
    }

    for(int i = 0; i < num; i++)
    {
        cout << infoArray[i].ip << ":" << infoArray[i].URI << endl;
    }

    cout << "-------------------------------------\n";

    delete[] infoArray;

    test();

    //cout << "\nSearch again\n\n";
    //system("pause");

    //if(-1 == searchDev())
    //{
    //    cout << "search device failed\n";
    //    system("pause");
    //    return -1;
    //}

    //num = getNumOfOnvifDev();

    //infoArray = new deviceInfoArray[num];

    //if(-1 == num)
    //{
    //    cout << "getNumOfOnvifDev failed\n";
    //    system("pause");
    //    return -1;
    //}

    //cout << "getNumOfOnvifDev: " << num << endl;

    //if(-1 == getAllDevURI(infoArray, num))
    //{
    //    cout << "getAllDevURI failed\n";
    //    system("pause");
    //    return -1;
    //}

    //for(int i = 0; i < num; i++)
    //{
    //    cout << infoArray[i].ip << ":" << infoArray[i].URI << endl;
    //}

    //cout << "-------------------------------------\n";
    //cout << "\nresetDll\n\n";

    //if(-1 == resetDll())
    //{
    //    cout << "resetDll failed\n";
    //    system("pause");
    //    return -1;
    //}

    //cout << "getNumOfOnvifDev: " << getNumOfOnvifDev() << endl;

    //delete[] infoArray;

    delete URI;

    if(-1 == uninitDll())
    {
        cout << "uninitDll failed\n";
        system("pause");
        return -1;
    }
    system("pause");
    return 0;
}

