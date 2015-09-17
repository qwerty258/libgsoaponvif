#include "receiveThreads.h"
#include "errorHandling.h"
#include <tchar.h>
#include <cstdlib>
#include <vector>
using namespace std;

typedef struct _receivedData
{
    IN_ADDR endPointAddr;
    BOOL forDelete;
    char* data;
}receivedData;

typedef struct _receiveThreadParameter
{
    SOCKET* socketForProbe;
    BOOL* bLoop;
    vector<receivedData*>* receivedDataList;
}receiveThreadParameter;

DWORD WINAPI receiveProbeMatchThread(LPVOID lpParam)
{
    receiveThreadParameter* parameter = static_cast<receiveThreadParameter*>(lpParam);

    sockaddr_in receivedFrom;
    int fromlen = sizeof(sockaddr_in);
    int bytesReceived;
    receivedData* pReceivedData = NULL;

    while((*parameter->bLoop))
    {
        pReceivedData = (receivedData*)malloc(sizeof(receivedData));
        if(NULL == pReceivedData)
        {
            handleError(_T("malloc"), _T(__FILE__), __LINE__);
            break;
        }
        memset(pReceivedData, 0x0, sizeof(receivedData));
        pReceivedData->data = (char*)malloc(USHRT_MAX);
        if(NULL == pReceivedData->data)
        {
            handleError(_T("malloc"), _T(__FILE__), __LINE__);
            break;
        }
        memset(pReceivedData->data, 0x0, USHRT_MAX);

        bytesReceived = recvfrom((*parameter->socketForProbe), pReceivedData->data, USHRT_MAX, 0, (sockaddr*)&receivedFrom, &fromlen);
        if(SOCKET_ERROR == bytesReceived)
        {
            if(NULL != pReceivedData)
            {
                if(NULL != pReceivedData->data)
                {
                    free(pReceivedData->data);
                }
                free(pReceivedData);
                pReceivedData = NULL;
            }
            break;
        }
        else
        {
            pReceivedData->endPointAddr.S_un = receivedFrom.sin_addr.S_un;
            parameter->receivedDataList->push_back(pReceivedData);
        }
    }

    return 0;
}
