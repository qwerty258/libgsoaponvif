// multiThreadTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <OnvifOperation.h>
#pragma comment(lib,"OnvifOperation.lib")

DWORD WINAPI getAllDeviceURI(LPVOID lpParameter)
{
    std::cout << "Thread getAllDeviceURI begin\n";

    int num = get_number_of_IPCs();

    if(-1 == num)
    {
        std::cout << "Thread getAllDeviceURI get_number_of_IPCs failed\n";
        std::cout << "Thread getAllDeviceURI end\n";
        return num;
    }

    IPC_URI* IPC_URI_array = new IPC_URI[num];

    int result = get_all_IPC_URIs(IPC_URI_array, num);

    if(-1 == result)
    {
        std::cout << "Thread getAllDeviceURI get_all_IPC_URIs failed\n";
        std::cout << "Thread getAllDeviceURI end\n";
        delete[] IPC_URI_array;
        return num;
    }

    for(int i = 0; i < num; i++)
    {
        std::cout << IPC_URI_array[i].ip << ':' << IPC_URI_array[i].URI << std::endl;
    }

    std::cout << "Thread getAllDeviceURI end\n";

    delete[] IPC_URI_array;

    return num;
}

DWORD WINAPI searchDevice(LPVOID lpParameter)
{
    std::cout << "Thread searchDevice begin\n";

    int  num = search_ONVIF_device(1);
    if(-1 == num)
    {
        std::cout << "Thread searchDevice searchDev failed\n";
        std::cout << "Thread searchDevice end\n";
        return num;
    }

    std::cout << "Thread searchDevice end\n";

    return num;
}

DWORD WINAPI getURIbyIP(LPVOID lpParameter)
{
    std::cout << "Thread getURIbyIP begin\n";

    char* URI = new char[256];

    int num = get_IPC_URI_according_to_IP("192.168.10.142", strlen("192.168.10.142") + 1, URI, 256, "admin", "12345");

    if(-1 == num)
    {
        std::cout << "Thread getURIbyIP get_IPC_URI_according_to_IP failed\n";
        std::cout << "Thread getURIbyIP end\n";
        delete[] URI;
        return num;
    }

    std::cout << URI << std::endl;

    std::cout << "Thread getURIbyIP end\n";

    delete URI;

    return num;
}

DWORD WINAPI getVideoInfoByIP(LPVOID lpParameter)
{
    std::cout << "Thread getNumOfProfilesByIP begin\n";

    int num = get_number_of_IPC_profiles_according_to_IP("192.168.10.142", strlen("192.168.10.142") + 1, "admin", "12345");

    if(-1 == num)
    {
        std::cout << "Thread getNumOfProfilesByIP get_number_of_IPC_profiles_according_to_IP failed\n";
        std::cout << "Thread getNumOfProfilesByIP end\n";
        return num;
    }

    IPC_profiles* IPC_profiles_array = new IPC_profiles[num];

    int result = get_IPC_profiles_according_to_IP("192.168.10.142", strlen("192.168.10.142") + 1, IPC_profiles_array, "admin", "12345");

    if(-1 == result)
    {
        std::cout << "Thread getNumOfProfilesByIP get_IPC_profiles_according_to_IP failed\n";
        std::cout << "Thread getNumOfProfilesByIP end\n";
        delete[] IPC_profiles_array;
        return num;
    }

    std::cout << "192.168.10.142: " << num << std::endl;

    for(int i = 0; i < num; i++)
    {
        std::cout << "-------------" << i << "---------------\n";
        switch(IPC_profiles_array[i].encoding)
        {
            case videoEncoding__JPEG:
                std::cout << "Encoding: JPEG\n";
                break;
            case videoEncoding__MPEG4:
                std::cout << "Encoding: MPEG4\n";
                break;
            case videoEncoding__H264:
                std::cout << "Encoding: H264\n";
                break;
            default:
                std::cout << "Encoding: Unknow\n";
                break;
        }
        std::cout << "Width: " << IPC_profiles_array[i].width << ' ' << "Height: " << IPC_profiles_array[i].height << std::endl;
        std::cout << "FrameRateLimit: " << IPC_profiles_array[i].frame << std::endl;
        std::cout << "URI: " << IPC_profiles_array[i].URI << std::endl;
    }

    std::cout << "Thread getNumOfProfilesByIP end\n";

    delete[] IPC_profiles_array;

    return result;
}

int _tmain(int argc, _TCHAR* argv[])
{
    init_DLL();

    HANDLE handleArray[5];

    handleArray[1] = CreateThread(NULL, 0, searchDevice, NULL, 0, NULL);
    Sleep(1000);
    handleArray[0] = CreateThread(NULL, 0, getAllDeviceURI, NULL, 0, NULL);
    handleArray[2] = CreateThread(NULL, 0, getURIbyIP, NULL, 0, NULL);
    handleArray[3] = CreateThread(NULL, 0, getVideoInfoByIP, NULL, 0, NULL);
    handleArray[4] = CreateThread(NULL, 0, searchDevice, NULL, 0, NULL);

    WaitForMultipleObjects(5, handleArray, TRUE, INFINITE);

    uninit_DLL();

    system("pause");

    return 0;
}

