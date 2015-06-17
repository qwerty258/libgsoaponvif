// getVideoInfo.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <OnvifOperation.h>

#pragma comment(lib,"OnvifOperation.lib")

#include <iostream>
using namespace std;

int _tmain(int argc, _TCHAR* argv[])
{
    system("pause");

    init_DLL();
    search_ONVIF_device(1);
    int num = get_number_of_IPCs();

    IPC_URI* IPC_URI_array = new IPC_URI[num];

    get_all_IPC_URIs(IPC_URI_array, num);

    for(int i = 0; i < num; i++)
    {
        cout << IPC_URI_array[i].ip << ":" << IPC_URI_array[i].URI << endl;
    }


    cout << "-------------------------------------\n";

    num = get_number_of_IPC_profiles_according_to_IP(IPC_URI_array[0].ip, strlen(IPC_URI_array[0].ip) + 1, "admin", "12345");

    if(-1 == num)
    {
        cout << "File: " << __FILE__ << " Line: " << __LINE__ << ' ' << "getNumOfProfilesFromIP error" << endl;
    }

    IPC_profiles* IPC_profiles_array = new IPC_profiles[num];

    get_IPC_profiles_according_to_IP(IPC_URI_array[0].ip, strlen(IPC_URI_array[0].ip) + 1, IPC_profiles_array, "admin", "12345");

    cout << IPC_URI_array[0].ip << ':' << endl;

    for(int i = 0; i < num; i++)
    {
        cout << "-------------" << i << "---------------\n";
        switch(IPC_profiles_array[i].encoding)
        {
            case videoEncoding__JPEG:
                cout << "Encoding: JPEG\n";
                break;
            case videoEncoding__MPEG4:
                cout << "Encoding: MPEG4\n";
                break;
            case videoEncoding__H264:
                cout << "Encoding: H264\n";
                break;
            default:
                cout << "Encoding: Unknow\n";
                break;
        }
        cout << "Width: " << IPC_profiles_array[i].width << ' ' << "Height: " << IPC_profiles_array[i].height << endl;
        cout << "FrameRateLimit: " << IPC_profiles_array[i].frame_rate_limit << endl;
        cout << "URI: " << IPC_profiles_array[i].URI << endl;
    }

    delete[] IPC_URI_array;
    IPC_URI_array = NULL;

    delete[] IPC_profiles_array;
    IPC_profiles_array = NULL;

    uninit_DLL();

    system("pause");

    return 0;
}

