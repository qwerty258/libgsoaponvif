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

    initDll();
    searchDev(1);
    int num = getNumOfOnvifDev();

    deviceInfo* deviceInfoArray = new deviceInfo[num];

    getAllDevURL(deviceInfoArray, num);

    for(int i = 0; i < num; i++)
    {
        cout << deviceInfoArray[i].ip << ":" << deviceInfoArray[i].URI << endl;
    }


    cout << "-------------------------------------\n";

    num = getNumOfProfilesFromIP(deviceInfoArray[0].ip, strlen(deviceInfoArray[0].ip) + 1, "admin", "12345");

    if(-1 == num)
    {
        cout << "File: " << __FILE__ << " Line: " << __LINE__ << ' ' << "getNumOfProfilesFromIP error" << endl;
    }

    videoInfo* videoInfoArray = new videoInfo[num];

    getVideoInfoFromIP(deviceInfoArray[0].ip, strlen(deviceInfoArray[0].ip) + 1, videoInfoArray, "admin", "12345");

    cout << deviceInfoArray[0].ip << ':' << endl;

    for(int i = 0; i < num; i++)
    {
        cout << "-------------" << i << "---------------\n";
        switch(videoInfoArray[i].encoding)
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
        cout << "Width: " << videoInfoArray[i].width << ' ' << "Height: " << videoInfoArray[i].height << endl;
        cout << "FrameRateLimit: " << videoInfoArray[i].frame << endl;
        cout << "URI: " << videoInfoArray[i].URI << endl;
    }

    delete[] videoInfoArray;
    videoInfoArray = NULL;

    delete[] deviceInfoArray;
    deviceInfoArray = NULL;

    uninitDll();

    system("pause");

    return 0;
}

