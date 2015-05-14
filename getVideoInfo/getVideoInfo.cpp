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
    searchDev();
    int num = getNumOfOnvifDev();

    deviceInfoArray* infoArray = new deviceInfoArray[num];

    getAllDevURI(infoArray, num);

    for(int i = 0; i < num; i++)
    {
        cout << infoArray[i].ip << ":" << infoArray[i].URI << endl;
    }


    cout << "-------------------------------------\n";

    num = getNumOfProfilesFromIP(infoArray[0].ip, strlen(infoArray[0].ip) + 1, "admin", "12345");

    videoNode* videoNodeArray = new videoNode[num];

    getVideoInfoFromIP(infoArray[0].ip, strlen(infoArray[0].ip) + 1, videoNodeArray, "admin", "12345");

    cout << infoArray[0].ip << ':' << endl;

    for(int i = 0; i < num; i++)
    {
        cout << "-------------" << i << "---------------\n";
        switch(videoNodeArray[i].encoding)
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
        cout << "Width: " << videoNodeArray[i].width << ' ' << "Height: " << videoNodeArray[i].height << endl;
        cout << "FrameRateLimit: " << videoNodeArray[i].frame << endl;
        cout << "URI: " << videoNodeArray[i].URI << endl;
    }

    delete[] videoNodeArray;
    videoNodeArray = NULL;

    delete[] infoArray;
    infoArray = NULL;

    uninitDll();

    system("pause");

    return 0;
}

