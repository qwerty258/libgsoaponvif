// multiThreadTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <OnvifOperation.h>
#pragma comment(lib,"OnvifOperation.lib")

DWORD WINAPI getAllDeviceURI(LPVOID lpParameter)
{
    std::cout << "Thread getAllDeviceURI begin\n";

    int num = getNumOfOnvifDev();

    deviceInfoArray* infoArray = new deviceInfoArray[num];

    int result = getAllDevURI(infoArray, num);

    if(-1 == result)
    {
        std::cout << "getAllDevURI failed\n";
    }

    for(int i = 0; i < num; i++)
    {
        std::cout << infoArray[i].ip << ':' << infoArray[i].URI << std::endl;
    }

    std::cout << "Thread getAllDeviceURI end\n";

    delete[] infoArray;

    return num;
}

DWORD WINAPI searchDevice(LPVOID lpParameter)
{
    int num;

    std::cout << "Thread searchDevice begin\n";

    for(size_t i = 0; i < 20; i++)
    {
        num = searchDev(1);

        if(-1 == num)
        {
            std::cout << "searchDev failed\n";
        }
    }

    std::cout << "Thread searchDevice end\n";

    return num;
}

DWORD WINAPI getURIbyIP(LPVOID lpParameter)
{
    std::cout << "Thread getURIbyIP begin\n";

    char* URI = new char[256];

    int num = getURIFromIP("192.168.10.142", strlen("192.168.10.142") + 1, URI, 256, "admin", "12345");

    std::cout << URI << std::endl;

    if(-1 == num)
    {
        std::cout << "getURIFromIP failed\n";
    }

    std::cout << "Thread getURIbyIP end\n";

    delete URI;

    return num;
}

DWORD WINAPI getVideoInfoByIP(LPVOID lpParameter)
{
    std::cout << "Thread getNumOfProfilesByIP begin\n";

    int num = getNumOfProfilesFromIP("192.168.10.142", strlen("192.168.10.142") + 1, "admin", "12345");

    if(-1 == num)
    {
        std::cout << "getNumOfProfilesFromIP failed\n";
        std::cout << "Thread getNumOfProfilesByIP end\n";
        return num;
    }

    videoNode* videoInfoArray = new videoNode[num];

    int result = getVideoInfoFromIP("192.168.10.142", strlen("192.168.10.142") + 1, videoInfoArray, "admin", "12345");

    std::cout << "192.168.10.142: " << num << std::endl;

    for(int i = 0; i < num; i++)
    {
        std::cout << "-------------" << i << "---------------\n";
        switch(videoInfoArray[i].encoding)
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
        std::cout << "Width: " << videoInfoArray[i].width << ' ' << "Height: " << videoInfoArray[i].height << std::endl;
        std::cout << "FrameRateLimit: " << videoInfoArray[i].frame << std::endl;
        std::cout << "URI: " << videoInfoArray[i].URI << std::endl;
    }

    std::cout << "Thread getNumOfProfilesByIP end\n";

    delete[] videoInfoArray;

    return result;
}

int _tmain(int argc, _TCHAR* argv[])
{
    initDll();

    HANDLE handleArray[5];

    handleArray[1] = CreateThread(NULL, 0, searchDevice, NULL, 0, NULL);
    handleArray[0] = CreateThread(NULL, 0, getAllDeviceURI, NULL, 0, NULL);
    handleArray[2] = CreateThread(NULL, 0, getURIbyIP, NULL, 0, NULL);
    handleArray[3] = CreateThread(NULL, 0, getVideoInfoByIP, NULL, 0, NULL);

    WaitForMultipleObjects(3, handleArray, TRUE, INFINITE);

    uninitDll();
    return 0;
}

