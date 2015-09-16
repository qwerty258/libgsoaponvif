#include "xmlParser.h"
#include "errorHandling.h"
#include <tchar.h>
#include <WS2tcpip.h>
#include <objbase.h>
#include <comutil.h>
#include <MsXml6.h>
#include <vector>
#include <string>
using namespace std;

typedef struct _receivedData
{
    IN_ADDR endPointAddr;
    BOOL forDelete;
    char* data;
}receivedData;

IXMLDOMNode* findNode(IXMLDOMNode* pIXMLDOMNode, char* pSzElementName)
{
    IXMLDOMNode* pIXMLDOMNodeTemp = NULL;
    IXMLDOMNodeList* pIXMLDOMNodeList = NULL;
    long length = 0;
    long releaseCount = 0;
    HRESULT hResult = 0;
    BSTR nodeName = NULL;
    char* szNodeName = NULL;

    hResult = pIXMLDOMNode->get_childNodes(&pIXMLDOMNodeList);
    if(FAILED(hResult))
    {
        goto findNodeCleanUp;
    }

    hResult = pIXMLDOMNodeList->get_length(&length);
    if(FAILED(hResult))
    {
        goto findNodeCleanUp;
    }

    for(long i = 0; i < length; i++)
    {
        pIXMLDOMNodeList->get_item(i, &pIXMLDOMNodeTemp);
        pIXMLDOMNodeTemp->get_nodeName(&nodeName);
        szNodeName = _com_util::ConvertBSTRToString(nodeName);

        SysFreeString(nodeName);
        nodeName = NULL;

        if(NULL != strstr(szNodeName, pSzElementName))
        {
            delete[] szNodeName;
            goto findNodeCleanUp;
        }

        delete[] szNodeName;

        if(NULL != pIXMLDOMNodeTemp)
        {
            do
            {
                releaseCount = pIXMLDOMNodeTemp->Release();
            } while(releaseCount > 0);
            pIXMLDOMNodeTemp = NULL;
        }
    }

findNodeCleanUp:

    if(NULL != pIXMLDOMNodeList)
    {
        do
        {
            releaseCount = pIXMLDOMNodeList->Release();
        } while(releaseCount > 0);
        pIXMLDOMNodeList = NULL;
    }

    return pIXMLDOMNodeTemp;
}

void getProbeMachXaddr(char* buffer, char* xml)
{
    HRESULT hResult = 0;
    long releaseResult;
    VARIANT_BOOL varStatus;

    BSTR bstrXMLInMemory = NULL;
    BSTR bstrError = NULL;
    BSTR bstrURI = NULL;
    char* szURI = NULL;

    IXMLDOMDocument* pIXMLDOMDocument = NULL;
    IXMLDOMParseError* pIXMLDOMParseError = NULL;
    IXMLDOMNode* pIXMLDOMNodeTemp = NULL;
    IXMLDOMNode* pIXMLDOMNodeFound = NULL;

    hResult = CoCreateInstance(__uuidof(DOMDocument60), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pIXMLDOMDocument));
    if(FAILED(hResult))
    {
        goto getProbeMachXaddrCleanUp;
    }

    hResult = pIXMLDOMDocument->put_async(VARIANT_FALSE);
    if(FAILED(hResult))
    {
        goto getProbeMachXaddrCleanUp;
    }

    hResult = pIXMLDOMDocument->put_validateOnParse(VARIANT_FALSE);
    if(FAILED(hResult))
    {
        goto getProbeMachXaddrCleanUp;
    }

    hResult = pIXMLDOMDocument->put_resolveExternals(VARIANT_FALSE);
    if(FAILED(hResult))
    {
        goto getProbeMachXaddrCleanUp;
    }

    bstrXMLInMemory = _com_util::ConvertStringToBSTR(xml);
    hResult = pIXMLDOMDocument->loadXML(bstrXMLInMemory, &varStatus);
    if(FAILED(hResult))
    {
        goto getProbeMachXaddrCleanUp;
    }
    if(varStatus != VARIANT_TRUE)
    {
        hResult = pIXMLDOMDocument->get_parseError(&pIXMLDOMParseError);
        if(FAILED(hResult))
        {
            goto getProbeMachXaddrCleanUp;
        }

        hResult = pIXMLDOMParseError->get_reason(&bstrError);
        if(FAILED(hResult))
        {
            goto getProbeMachXaddrCleanUp;
        }

        handleError(bstrError, _T(__FILE__), __LINE__);
    }

    pIXMLDOMNodeFound = findNode(pIXMLDOMDocument, "Envelope");
    if(NULL == pIXMLDOMNodeFound)
    {
        goto getProbeMachXaddrCleanUp;
    }

    pIXMLDOMNodeTemp = pIXMLDOMNodeFound;

    pIXMLDOMNodeFound = findNode(pIXMLDOMNodeTemp, "Body");

    do
    {
        releaseResult = pIXMLDOMNodeTemp->Release();
    } while(releaseResult > 0);
    pIXMLDOMNodeTemp = NULL;

    if(NULL == pIXMLDOMNodeFound)
    {
        goto getProbeMachXaddrCleanUp;
    }
    pIXMLDOMNodeTemp = pIXMLDOMNodeFound;

    pIXMLDOMNodeFound = findNode(pIXMLDOMNodeTemp, "ProbeMatches");

    do
    {
        releaseResult = pIXMLDOMNodeTemp->Release();
    } while(releaseResult > 0);
    pIXMLDOMNodeTemp = NULL;

    if(NULL == pIXMLDOMNodeFound)
    {
        goto getProbeMachXaddrCleanUp;
    }

    pIXMLDOMNodeTemp = pIXMLDOMNodeFound;

    pIXMLDOMNodeFound = findNode(pIXMLDOMNodeTemp, "ProbeMatch");

    do
    {
        releaseResult = pIXMLDOMNodeTemp->Release();
    } while(releaseResult > 0);
    pIXMLDOMNodeTemp = NULL;

    if(NULL == pIXMLDOMNodeFound)
    {
        goto getProbeMachXaddrCleanUp;
    }

    pIXMLDOMNodeTemp = pIXMLDOMNodeFound;

    pIXMLDOMNodeFound = findNode(pIXMLDOMNodeTemp, "XAddrs");

    do
    {
        releaseResult = pIXMLDOMNodeTemp->Release();
    } while(releaseResult > 0);
    pIXMLDOMNodeTemp = NULL;

    if(NULL == pIXMLDOMNodeFound)
    {
        goto getProbeMachXaddrCleanUp;
    }

    hResult = pIXMLDOMNodeFound->get_text(&bstrURI);
    if(FAILED(hResult))
    {
        goto getProbeMachXaddrCleanUp;
    }

    szURI = _com_util::ConvertBSTRToString(bstrURI);

    strncpy(buffer, szURI, 256);

    if(NULL != szURI)
    {
        delete[] szURI;
        szURI = NULL;
    }

getProbeMachXaddrCleanUp:

    if(NULL != pIXMLDOMNodeFound)
    {
        do
        {
            releaseResult = pIXMLDOMNodeFound->Release();
        } while(releaseResult > 0);
        pIXMLDOMNodeFound = NULL;
    }

    if(NULL != pIXMLDOMParseError)
    {
        do
        {
            releaseResult = pIXMLDOMParseError->Release();
        } while(releaseResult > 0);
        pIXMLDOMParseError = NULL;
    }

    if(NULL != pIXMLDOMDocument)
    {
        do
        {
            releaseResult = pIXMLDOMDocument->Release();
        } while(releaseResult > 0);
        pIXMLDOMDocument = NULL;
    }

    SysFreeString(bstrError);
    SysFreeString(bstrXMLInMemory);
    SysFreeString(bstrURI);
}

void parseDiscoveredDeviceXML(onvif_device_list* p_onvif_device_list, void* receivedDataList)
{
    vector<receivedData*>* pReceivedDataList = static_cast<vector<receivedData*>*>(receivedDataList);

    // set all to not duplicated
    for(size_t i = 0; i < p_onvif_device_list->number_of_onvif_devices; ++i)
    {
        p_onvif_device_list->p_onvif_devices[i].duplicated = false;
    }

    char ipTemp[50];
    char URI[256];

    size_t size = pReceivedDataList->size();
    onvif_device* p_onvif_device_temp;
    int result;
    size_t i;
    size_t j;

    // add new device into list
    for(i = 0; i < size; ++i)
    {
        memset(ipTemp, 0x0, 50);
        memset(URI, 0x0, 256);
        result = _snprintf_s(ipTemp, 50, _TRUNCATE, "%d.%d.%d.%d", (*pReceivedDataList)[i]->endPointAddr.S_un.S_un_b.s_b1, (*pReceivedDataList)[i]->endPointAddr.S_un.S_un_b.s_b2, (*pReceivedDataList)[i]->endPointAddr.S_un.S_un_b.s_b3, (*pReceivedDataList)[i]->endPointAddr.S_un.S_un_b.s_b4);
        if(-1 == result)
        {
            continue;
        }

        getProbeMachXaddr(URI, (*pReceivedDataList)[i]->data);
        if(5 > strnlen(URI, 256))
        {
            continue;
        }

        if(NULL == strstr(URI, "onvif"))
        {
            continue;
        }

        // find device already in the list and set to duplicated
        for(j = 0; j < p_onvif_device_list->number_of_onvif_devices; ++j)
        {
            if(0 == strncmp(ipTemp, p_onvif_device_list->p_onvif_devices[j].IPv4, 16))
            {
                p_onvif_device_list->p_onvif_devices[j].duplicated = true;
                break;
            }
        }

        // if not found, it is new device
        if(j == p_onvif_device_list->number_of_onvif_devices)
        {
            p_onvif_device_list->number_of_onvif_devices += 1;
            p_onvif_device_temp = (onvif_device*)realloc(p_onvif_device_list->p_onvif_devices, p_onvif_device_list->number_of_onvif_devices * sizeof(onvif_device));
            if(NULL == p_onvif_device_temp)
            {
                exit(-1);
            }
            else
            {
                p_onvif_device_list->p_onvif_devices = p_onvif_device_temp;
                memset(&p_onvif_device_list->p_onvif_devices[p_onvif_device_list->number_of_onvif_devices - 1], 0x0, sizeof(onvif_device));

                strncpy(
                    p_onvif_device_list->p_onvif_devices[p_onvif_device_list->number_of_onvif_devices - 1].service_address_device_service.xaddr,
                    URI,
                    256);

                strncpy(
                    p_onvif_device_list->p_onvif_devices[p_onvif_device_list->number_of_onvif_devices - 1].IPv4,
                    ipTemp,
                    17);
                p_onvif_device_list->p_onvif_devices[p_onvif_device_list->number_of_onvif_devices - 1].duplicated = true;
            }
        }
    }

    // remove old disappeared device
    for(i = 0; i < p_onvif_device_list->number_of_onvif_devices; ++i)
    {
        if(!p_onvif_device_list->p_onvif_devices[i].duplicated)
        {
            // remove profiles array
            if(NULL != p_onvif_device_list->p_onvif_devices[i].p_onvif_ipc_profiles)
            {
                free(p_onvif_device_list->p_onvif_devices[i].p_onvif_ipc_profiles);
                p_onvif_device_list->p_onvif_devices[i].p_onvif_ipc_profiles = NULL;
            }

            // remove NVR receivers array
            if(NULL != p_onvif_device_list->p_onvif_devices[i].p_onvif_NVR_receivers)
            {
                free(p_onvif_device_list->p_onvif_devices[i].p_onvif_NVR_receivers);
                p_onvif_device_list->p_onvif_devices[i].p_onvif_NVR_receivers = NULL;
            }

            // move elements behind forward
            for(j = i; j + 1 < p_onvif_device_list->number_of_onvif_devices; ++j)
            {
                p_onvif_device_list->p_onvif_devices[j] = p_onvif_device_list->p_onvif_devices[j + 1];
            }

            --i;
            --(p_onvif_device_list->number_of_onvif_devices);
        }
    }
}