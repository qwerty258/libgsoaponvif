// testProbeByMyself.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <WS2tcpip.h>
#include <WinSock2.h>

typedef struct _receiveThreadParameter
{
    SOCKET* socketForProbe;
    BOOL* bLoop;
}receiveThreadParameter;

DWORD WINAPI receiveThread(LPVOID lpParam);

int _tmain(int argc, _TCHAR* argv[])
{
    WSADATA wsaData;
    int result;

    system("pause");

    result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if(0 != result)
    {
        _tprintf(_T("WSAStartup error, code: %d"), WSAGetLastError());
        exit(0);
    }

    SOCKET socketForProbe = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(INVALID_SOCKET == socketForProbe)
    {
        _tprintf(_T("socket error, code: %d"), WSAGetLastError());
        exit(0);
    }

    sockaddr_in sockaddrClient;
    memset(&sockaddrClient, 0x0, sizeof(sockaddr_in));
    sockaddrClient.sin_addr.s_addr = htonl(INADDR_ANY);
    sockaddrClient.sin_family = AF_INET;
    sockaddrClient.sin_port = htons(0);

    sockaddr_in sockaddrMulticastAddrForOnvif;
    memset(&sockaddrMulticastAddrForOnvif, 0x0, sizeof(sockaddr_in));
    InetPton(AF_INET, _T("239.255.255.250"), &sockaddrMulticastAddrForOnvif.sin_addr.s_addr);
    sockaddrMulticastAddrForOnvif.sin_family = AF_INET;
    sockaddrMulticastAddrForOnvif.sin_port = htons(3702);

    result = bind(socketForProbe, (struct sockaddr*)&sockaddrClient, sizeof(sockaddr_in));
    if(0 != result)
    {
        _tprintf(_T("bind error, code: %d"), WSAGetLastError());
        exit(0);
    }

    DWORD timeOut = 5000;

    result = setsockopt(socketForProbe, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeOut, sizeof(DWORD));
    if(0 != result)
    {
        _tprintf(_T("setsockopt error, code: %d"), WSAGetLastError());
        exit(0);
    }


    char* pProbeMessage = (char*)malloc(2048);
    if(NULL == pProbeMessage)
    {
        _tprintf(_T("malloc error"));
        exit(0);
    }


    //NetworkVideoTransmitter
    //onvif://www.onvif.org


    receiveThreadParameter parameter;
    BOOL loop = TRUE;
    DWORD threadID;
    parameter.socketForProbe = &socketForProbe;
    parameter.bLoop = &loop;

    HANDLE hThread = CreateThread(NULL, 0, receiveThread, &parameter, 0, &threadID);
    if(NULL == hThread)
    {
        _tprintf(_T("CreateThread error, code: %d"), GetLastError());
        exit(0);
    }

    result = _snprintf_s(pProbeMessage, 2048, _TRUNCATE, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n<SOAP-ENV:Envelope xmlns:SOAP-ENV=\"http://www.w3.org/2003/05/soap-envelope\" xmlns:SOAP-ENC=\"http://www.w3.org/2003/05/soap-encoding\" xmlns:wsa=\"http://schemas.xmlsoap.org/ws/2004/08/addressing\" xmlns:wsdd=\"http://schemas.xmlsoap.org/ws/2005/04/discovery\"><SOAP-ENV:Header><wsa:MessageID>urn:uuid:759a9a95-610f-40a9-bf26-fe2c2836ed50</wsa:MessageID><wsa:To SOAP-ENV:mustUnderstand=\"true\">urn:schemas-xmlsoap-org:ws:2005:04:discovery</wsa:To><wsa:Action SOAP-ENV:mustUnderstand=\"true\">http://schemas.xmlsoap.org/ws/2005/04/discovery/Probe</wsa:Action></SOAP-ENV:Header><SOAP-ENV:Body><wsdd:Probe><wsdd:Types>NetworkVideoTransmitter</wsdd:Types><wsdd:Scopes></wsdd:Scopes></wsdd:Probe></SOAP-ENV:Body></SOAP-ENV:Envelope>");
    if(-1 == result)
    {
        _tprintf(_T("_snprintf_s error"));
        exit(0);
    }

    result = sendto(socketForProbe, pProbeMessage, result, 0, (sockaddr*)&sockaddrMulticastAddrForOnvif, sizeof(sockaddr_in));
    if(SOCKET_ERROR == result)
    {
        _tprintf(_T("sendto error, code: %d"), WSAGetLastError());
        exit(0);
    }

    Sleep(5000);

    loop = FALSE;

    WaitForMultipleObjects(1, &hThread, TRUE, INFINITE);

    CloseHandle(hThread);





    hThread = CreateThread(NULL, 0, receiveThread, &parameter, 0, &threadID);
    if(NULL == hThread)
    {
        _tprintf(_T("CreateThread error, code: %d"), GetLastError());
        exit(0);
    }

    result = _snprintf_s(pProbeMessage, 2048, _TRUNCATE, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n<SOAP-ENV:Envelope xmlns:SOAP-ENV=\"http://www.w3.org/2003/05/soap-envelope\" xmlns:SOAP-ENC=\"http://www.w3.org/2003/05/soap-encoding\" xmlns:wsa=\"http://schemas.xmlsoap.org/ws/2004/08/addressing\" xmlns:wsdd=\"http://schemas.xmlsoap.org/ws/2005/04/discovery\"><SOAP-ENV:Header><wsa:MessageID>urn:uuid:bc9fb550-1dd1-11b2-807c-c056e3fb5481</wsa:MessageID><wsa:To SOAP-ENV:mustUnderstand=\"true\">urn:schemas-xmlsoap-org:ws:2005:04:discovery</wsa:To><wsa:Action SOAP-ENV:mustUnderstand=\"true\">http://schemas.xmlsoap.org/ws/2005/04/discovery/Probe</wsa:Action></SOAP-ENV:Header><SOAP-ENV:Body><wsdd:Probe><wsdd:Types>Device</wsdd:Types><wsdd:Scopes></wsdd:Scopes></wsdd:Probe></SOAP-ENV:Body></SOAP-ENV:Envelope>");
    if(-1 == result)
    {
        _tprintf(_T("_snprintf_s error"));
        exit(0);
    }

    result = sendto(socketForProbe, pProbeMessage, result, 0, (sockaddr*)&sockaddrMulticastAddrForOnvif, sizeof(sockaddr_in));
    if(SOCKET_ERROR == result)
    {
        _tprintf(_T("sendto error, code: %d"), WSAGetLastError());
        exit(0);
    }


    Sleep(5000);

    loop = FALSE;

    WaitForMultipleObjects(1, &hThread, TRUE, INFINITE);

    CloseHandle(hThread);

    free(pProbeMessage);
    pProbeMessage = NULL;

    result = closesocket(socketForProbe);
    if(0 != result)
    {
        _tprintf(_T("closesocket error, code: %d"), WSAGetLastError());
        exit(0);
    }

    result = WSACleanup();
    if(0 != result)
    {
        _tprintf(_T("WSACleanup error, code: %d"), WSAGetLastError());
        exit(0);
    }

    system("pause");

    return 0;
}

DWORD WINAPI receiveThread(LPVOID lpParam)
{
    receiveThreadParameter* parameter = static_cast<receiveThreadParameter*>(lpParam);

    sockaddr_in receivedFrom;
    int fromlen = sizeof(sockaddr_in);
    int bytesReceived;

    char* buffer = (char*)malloc(USHRT_MAX);

    FILE* pFile;
    errno_t error = fopen_s(&pFile, "D:\\onvif_probe_result.txt", "wb");
    if(0 != error)
    {
        _tprintf(_T("fopen error, code: %d"), GetLastError());
    }

    while((*parameter->bLoop))
    {
        memset(buffer, 0x0, USHRT_MAX);

        bytesReceived = recvfrom((*parameter->socketForProbe), buffer, USHRT_MAX, 0, (sockaddr*)&receivedFrom, &fromlen);
        if(SOCKET_ERROR == bytesReceived)
        {
            _tprintf(_T("recvfrom error, code: %d\n"), WSAGetLastError());
        }
        else
        {
            fwrite(buffer, bytesReceived, 1, pFile);
            fwrite("\r\n\r\n\r\n", 6, 1, pFile);

            printf_s("%s\n\n\n", buffer);
        }
    }

    fclose(pFile);

    free(buffer);
    buffer = NULL;

    return 0;
}