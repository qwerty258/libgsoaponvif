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

void parseGetServicesResponse(onvif_device* p_onvif_device, void* receivedDataList)
{
    vector<receivedData*>* pReceivedDataList = static_cast<vector<receivedData*>*>(receivedDataList);

    HRESULT hResult;
    long releaseResult;
    VARIANT_BOOL varStatus;

    BSTR bstrXMLInMemory = NULL;
    BSTR bstrError = NULL;

    IXMLDOMDocument* pIXMLDOMDocument = NULL;
    IXMLDOMParseError* pIXMLDOMParseError = NULL;
    IXMLDOMNode* pIXMLDOMNodeTemp = NULL;
    IXMLDOMNode* pIXMLDOMNodeFound = NULL;

    long length = 0;

    BSTR bstrNamespace = NULL;
    BSTR bstrXAddr = NULL;
    BSTR bstrMajor = NULL;
    BSTR bstrMinor = NULL;
    char* szNamespace = NULL;
    char* szXAddr = NULL;
    char* szMajor = NULL;
    char* szMinor = NULL;

    IXMLDOMNodeList* pIXMLDOMNodeList = NULL;
    IXMLDOMNode* pNodeOfNamespace = NULL;
    IXMLDOMNode* pNodeOfXAddr = NULL;
    IXMLDOMNode* pNodeOfVersion = NULL;
    IXMLDOMNode* pNodeOfMajor = NULL;
    IXMLDOMNode* pNodeOfMinor = NULL;

    if(1 != pReceivedDataList->size())
    {
        return;
    }

    char* end;
    char* currentPosition = strstr((*pReceivedDataList)[0]->data, "Content-Length");
    if(NULL == currentPosition)
    {
        goto parseGetServicesResponseCleanUp;
    }

    currentPosition = strstr(currentPosition, ":");
    if(NULL == currentPosition)
    {
        goto parseGetServicesResponseCleanUp;
    }

    int contentLength = strtol(currentPosition + 1, &end, 10);

    currentPosition = strstr(currentPosition, "\r\n\r\n");
    if(NULL == currentPosition)
    {
        goto parseGetServicesResponseCleanUp;
    }

    currentPosition += 4;


    hResult = CoCreateInstance(__uuidof(DOMDocument60), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pIXMLDOMDocument));
    if(FAILED(hResult))
    {
        goto parseGetServicesResponseCleanUp;
    }

    hResult = pIXMLDOMDocument->put_async(VARIANT_FALSE);
    if(FAILED(hResult))
    {
        goto parseGetServicesResponseCleanUp;
    }

    hResult = pIXMLDOMDocument->put_validateOnParse(VARIANT_FALSE);
    if(FAILED(hResult))
    {
        goto parseGetServicesResponseCleanUp;
    }

    hResult = pIXMLDOMDocument->put_resolveExternals(VARIANT_FALSE);
    if(FAILED(hResult))
    {
        goto parseGetServicesResponseCleanUp;
    }

    bstrXMLInMemory = _com_util::ConvertStringToBSTR(currentPosition);
    hResult = pIXMLDOMDocument->loadXML(bstrXMLInMemory, &varStatus);
    if(FAILED(hResult))
    {
        goto parseGetServicesResponseCleanUp;
    }
    if(varStatus != VARIANT_TRUE)
    {
        hResult = pIXMLDOMDocument->get_parseError(&pIXMLDOMParseError);
        if(FAILED(hResult))
        {
            goto parseGetServicesResponseCleanUp;
        }

        hResult = pIXMLDOMParseError->get_reason(&bstrError);
        if(FAILED(hResult))
        {
            goto parseGetServicesResponseCleanUp;
        }

        handleError(bstrError, _T(__FILE__), __LINE__);
    }

    pIXMLDOMNodeFound = findNode(pIXMLDOMDocument, "Envelope");
    if(NULL == pIXMLDOMNodeFound)
    {
        goto parseGetServicesResponseCleanUp;
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
        goto parseGetServicesResponseCleanUp;
    }
    pIXMLDOMNodeTemp = pIXMLDOMNodeFound;

    pIXMLDOMNodeFound = findNode(pIXMLDOMNodeTemp, "GetServicesResponse");

    do
    {
        releaseResult = pIXMLDOMNodeTemp->Release();
    } while(releaseResult > 0);
    pIXMLDOMNodeTemp = NULL;

    if(NULL == pIXMLDOMNodeFound)
    {
        goto parseGetServicesResponseCleanUp;
    }

    hResult = pIXMLDOMNodeFound->get_childNodes(&pIXMLDOMNodeList);
    if(FAILED(hResult))
    {
        goto parseGetServicesResponseCleanUp;
    }

    hResult = pIXMLDOMNodeList->get_length(&length);
    if(FAILED(hResult))
    {
        goto parseGetServicesResponseCleanUp;
    }

    for(long i = 0; i < length; i++)
    {
        hResult = pIXMLDOMNodeList->get_item(i, &pIXMLDOMNodeTemp);
        if(FAILED(hResult))
        {
            break;
        }

        pNodeOfNamespace = findNode(pIXMLDOMNodeTemp, "Namespace");
        if(NULL != pNodeOfNamespace)
        {
            hResult = pNodeOfNamespace->get_text(&bstrNamespace);
            if(SUCCEEDED(hResult))
            {
                szNamespace = _com_util::ConvertBSTRToString(bstrNamespace);
                SysFreeString(bstrNamespace);
                bstrNamespace = NULL;

                // service_address_device_service begin
                if(NULL != strstr(szNamespace, "device/wsdl"))
                {
                    strncpy(p_onvif_device->service_address_device_service.namesapce, szNamespace, 256);
                    pNodeOfXAddr = findNode(pIXMLDOMNodeTemp, "XAddr");
                    if(NULL != pNodeOfXAddr)
                    {
                        hResult = pNodeOfXAddr->get_text(&bstrXAddr);
                        if(SUCCEEDED(hResult))
                        {
                            szXAddr = _com_util::ConvertBSTRToString(bstrXAddr);
                            SysFreeString(bstrXAddr);
                            bstrXAddr = NULL;
                            strncpy(p_onvif_device->service_address_device_service.xaddr, szXAddr, 256);
                            delete[] szXAddr;
                        }
                        do
                        {
                            releaseResult = pNodeOfXAddr->Release();
                        } while(releaseResult > 0);
                        pNodeOfXAddr = NULL;
                    }
                    pNodeOfVersion = findNode(pIXMLDOMNodeTemp, "Version");
                    if(NULL != pNodeOfVersion)
                    {
                        pNodeOfMajor = findNode(pNodeOfVersion, "Major");
                        if(NULL != pNodeOfMajor)
                        {
                            hResult = pNodeOfMajor->get_text(&bstrMajor);
                            if(SUCCEEDED(hResult))
                            {
                                szMajor = _com_util::ConvertBSTRToString(bstrMajor);
                                SysFreeString(bstrMajor);
                                p_onvif_device->service_address_device_service.major_version = strtol(szMajor, &end, 10);
                                delete[] szMajor;
                                szMajor = NULL;
                            }
                            do
                            {
                                releaseResult = pNodeOfMajor->Release();
                            } while(releaseResult > 0);
                            pNodeOfMajor = NULL;
                        }
                        pNodeOfMinor = findNode(pNodeOfVersion, "Minor");
                        if(NULL != pNodeOfMinor)
                        {
                            hResult = pNodeOfMinor->get_text(&bstrMinor);
                            if(SUCCEEDED(hResult))
                            {
                                szMinor = _com_util::ConvertBSTRToString(bstrMinor);
                                SysFreeString(bstrMinor);
                                p_onvif_device->service_address_device_service.minor_version = strtol(szMinor, &end, 10);
                                delete[] szMinor;
                                szMinor = NULL;
                            }
                            do
                            {
                                releaseResult = pNodeOfMinor->Release();
                            } while(releaseResult > 0);
                            pNodeOfMinor = NULL;
                        }
                        do
                        {
                            releaseResult = pNodeOfVersion->Release();
                        } while(releaseResult > 0);
                        pNodeOfVersion = NULL;
                    }
                }
                // service_address_device_service end

                // service_address_media begin
                if(NULL != strstr(szNamespace, "media/wsdl"))
                {
                    strncpy(p_onvif_device->service_address_media.namesapce, szNamespace, 256);
                    pNodeOfXAddr = findNode(pIXMLDOMNodeTemp, "XAddr");
                    if(NULL != pNodeOfXAddr)
                    {
                        hResult = pNodeOfXAddr->get_text(&bstrXAddr);
                        if(SUCCEEDED(hResult))
                        {
                            szXAddr = _com_util::ConvertBSTRToString(bstrXAddr);
                            SysFreeString(bstrXAddr);
                            bstrXAddr = NULL;
                            strncpy(p_onvif_device->service_address_media.xaddr, szXAddr, 256);
                            delete[] szXAddr;
                        }
                        do
                        {
                            releaseResult = pNodeOfXAddr->Release();
                        } while(releaseResult > 0);
                        pNodeOfXAddr = NULL;
                    }
                    pNodeOfVersion = findNode(pIXMLDOMNodeTemp, "Version");
                    if(NULL != pNodeOfVersion)
                    {
                        pNodeOfMajor = findNode(pNodeOfVersion, "Major");
                        if(NULL != pNodeOfMajor)
                        {
                            hResult = pNodeOfMajor->get_text(&bstrMajor);
                            if(SUCCEEDED(hResult))
                            {
                                szMajor = _com_util::ConvertBSTRToString(bstrMajor);
                                SysFreeString(bstrMajor);
                                p_onvif_device->service_address_media.major_version = strtol(szMajor, &end, 10);
                                delete[] szMajor;
                                szMajor = NULL;
                            }
                            do
                            {
                                releaseResult = pNodeOfMajor->Release();
                            } while(releaseResult > 0);
                            pNodeOfMajor = NULL;
                        }
                        pNodeOfMinor = findNode(pNodeOfVersion, "Minor");
                        if(NULL != pNodeOfMinor)
                        {
                            hResult = pNodeOfMinor->get_text(&bstrMinor);
                            if(SUCCEEDED(hResult))
                            {
                                szMinor = _com_util::ConvertBSTRToString(bstrMinor);
                                SysFreeString(bstrMinor);
                                p_onvif_device->service_address_media.minor_version = strtol(szMinor, &end, 10);
                                delete[] szMinor;
                                szMinor = NULL;
                            }
                            do
                            {
                                releaseResult = pNodeOfMinor->Release();
                            } while(releaseResult > 0);
                            pNodeOfMinor = NULL;
                        }
                        do
                        {
                            releaseResult = pNodeOfVersion->Release();
                        } while(releaseResult > 0);
                        pNodeOfVersion = NULL;
                    }
                }
                // service_address_media end

                // service_address_events begin
                if(NULL != strstr(szNamespace, "events/wsdl"))
                {
                    strncpy(p_onvif_device->service_address_events.namesapce, szNamespace, 256);
                    pNodeOfXAddr = findNode(pIXMLDOMNodeTemp, "XAddr");
                    if(NULL != pNodeOfXAddr)
                    {
                        hResult = pNodeOfXAddr->get_text(&bstrXAddr);
                        if(SUCCEEDED(hResult))
                        {
                            szXAddr = _com_util::ConvertBSTRToString(bstrXAddr);
                            SysFreeString(bstrXAddr);
                            bstrXAddr = NULL;
                            strncpy(p_onvif_device->service_address_events.xaddr, szXAddr, 256);
                            delete[] szXAddr;
                        }
                        do
                        {
                            releaseResult = pNodeOfXAddr->Release();
                        } while(releaseResult > 0);
                        pNodeOfXAddr = NULL;
                    }
                    pNodeOfVersion = findNode(pIXMLDOMNodeTemp, "Version");
                    if(NULL != pNodeOfVersion)
                    {
                        pNodeOfMajor = findNode(pNodeOfVersion, "Major");
                        if(NULL != pNodeOfMajor)
                        {
                            hResult = pNodeOfMajor->get_text(&bstrMajor);
                            if(SUCCEEDED(hResult))
                            {
                                szMajor = _com_util::ConvertBSTRToString(bstrMajor);
                                SysFreeString(bstrMajor);
                                p_onvif_device->service_address_events.major_version = strtol(szMajor, &end, 10);
                                delete[] szMajor;
                                szMajor = NULL;
                            }
                            do
                            {
                                releaseResult = pNodeOfMajor->Release();
                            } while(releaseResult > 0);
                            pNodeOfMajor = NULL;
                        }
                        pNodeOfMinor = findNode(pNodeOfVersion, "Minor");
                        if(NULL != pNodeOfMinor)
                        {
                            hResult = pNodeOfMinor->get_text(&bstrMinor);
                            if(SUCCEEDED(hResult))
                            {
                                szMinor = _com_util::ConvertBSTRToString(bstrMinor);
                                SysFreeString(bstrMinor);
                                p_onvif_device->service_address_events.minor_version = strtol(szMinor, &end, 10);
                                delete[] szMinor;
                                szMinor = NULL;
                            }
                            do
                            {
                                releaseResult = pNodeOfMinor->Release();
                            } while(releaseResult > 0);
                            pNodeOfMinor = NULL;
                        }
                        do
                        {
                            releaseResult = pNodeOfVersion->Release();
                        } while(releaseResult > 0);
                        pNodeOfVersion = NULL;
                    }
                }
                // service_address_events end

                // service_address_PTZ begin
                if(NULL != strstr(szNamespace, "ptz/wsdl"))
                {
                    strncpy(p_onvif_device->service_address_PTZ.namesapce, szNamespace, 256);
                    pNodeOfXAddr = findNode(pIXMLDOMNodeTemp, "XAddr");
                    if(NULL != pNodeOfXAddr)
                    {
                        hResult = pNodeOfXAddr->get_text(&bstrXAddr);
                        if(SUCCEEDED(hResult))
                        {
                            szXAddr = _com_util::ConvertBSTRToString(bstrXAddr);
                            SysFreeString(bstrXAddr);
                            bstrXAddr = NULL;
                            strncpy(p_onvif_device->service_address_PTZ.xaddr, szXAddr, 256);
                            delete[] szXAddr;
                        }
                        do
                        {
                            releaseResult = pNodeOfXAddr->Release();
                        } while(releaseResult > 0);
                        pNodeOfXAddr = NULL;
                    }
                    pNodeOfVersion = findNode(pIXMLDOMNodeTemp, "Version");
                    if(NULL != pNodeOfVersion)
                    {
                        pNodeOfMajor = findNode(pNodeOfVersion, "Major");
                        if(NULL != pNodeOfMajor)
                        {
                            hResult = pNodeOfMajor->get_text(&bstrMajor);
                            if(SUCCEEDED(hResult))
                            {
                                szMajor = _com_util::ConvertBSTRToString(bstrMajor);
                                SysFreeString(bstrMajor);
                                p_onvif_device->service_address_PTZ.major_version = strtol(szMajor, &end, 10);
                                delete[] szMajor;
                                szMajor = NULL;
                            }
                            do
                            {
                                releaseResult = pNodeOfMajor->Release();
                            } while(releaseResult > 0);
                            pNodeOfMajor = NULL;
                        }
                        pNodeOfMinor = findNode(pNodeOfVersion, "Minor");
                        if(NULL != pNodeOfMinor)
                        {
                            hResult = pNodeOfMinor->get_text(&bstrMinor);
                            if(SUCCEEDED(hResult))
                            {
                                szMinor = _com_util::ConvertBSTRToString(bstrMinor);
                                SysFreeString(bstrMinor);
                                p_onvif_device->service_address_PTZ.minor_version = strtol(szMinor, &end, 10);
                                delete[] szMinor;
                                szMinor = NULL;
                            }
                            do
                            {
                                releaseResult = pNodeOfMinor->Release();
                            } while(releaseResult > 0);
                            pNodeOfMinor = NULL;
                        }
                        do
                        {
                            releaseResult = pNodeOfVersion->Release();
                        } while(releaseResult > 0);
                        pNodeOfVersion = NULL;
                    }
                }
                // service_address_PTZ end

                // service_address_imaging begin
                if(NULL != strstr(szNamespace, "imaging/wsdl"))
                {
                    strncpy(p_onvif_device->service_address_imaging.namesapce, szNamespace, 256);
                    pNodeOfXAddr = findNode(pIXMLDOMNodeTemp, "XAddr");
                    if(NULL != pNodeOfXAddr)
                    {
                        hResult = pNodeOfXAddr->get_text(&bstrXAddr);
                        if(SUCCEEDED(hResult))
                        {
                            szXAddr = _com_util::ConvertBSTRToString(bstrXAddr);
                            SysFreeString(bstrXAddr);
                            bstrXAddr = NULL;
                            strncpy(p_onvif_device->service_address_imaging.xaddr, szXAddr, 256);
                            delete[] szXAddr;
                        }
                        do
                        {
                            releaseResult = pNodeOfXAddr->Release();
                        } while(releaseResult > 0);
                        pNodeOfXAddr = NULL;
                    }
                    pNodeOfVersion = findNode(pIXMLDOMNodeTemp, "Version");
                    if(NULL != pNodeOfVersion)
                    {
                        pNodeOfMajor = findNode(pNodeOfVersion, "Major");
                        if(NULL != pNodeOfMajor)
                        {
                            hResult = pNodeOfMajor->get_text(&bstrMajor);
                            if(SUCCEEDED(hResult))
                            {
                                szMajor = _com_util::ConvertBSTRToString(bstrMajor);
                                SysFreeString(bstrMajor);
                                p_onvif_device->service_address_imaging.major_version = strtol(szMajor, &end, 10);
                                delete[] szMajor;
                                szMajor = NULL;
                            }
                            do
                            {
                                releaseResult = pNodeOfMajor->Release();
                            } while(releaseResult > 0);
                            pNodeOfMajor = NULL;
                        }
                        pNodeOfMinor = findNode(pNodeOfVersion, "Minor");
                        if(NULL != pNodeOfMinor)
                        {
                            hResult = pNodeOfMinor->get_text(&bstrMinor);
                            if(SUCCEEDED(hResult))
                            {
                                szMinor = _com_util::ConvertBSTRToString(bstrMinor);
                                SysFreeString(bstrMinor);
                                p_onvif_device->service_address_imaging.minor_version = strtol(szMinor, &end, 10);
                                delete[] szMinor;
                                szMinor = NULL;
                            }
                            do
                            {
                                releaseResult = pNodeOfMinor->Release();
                            } while(releaseResult > 0);
                            pNodeOfMinor = NULL;
                        }
                        do
                        {
                            releaseResult = pNodeOfVersion->Release();
                        } while(releaseResult > 0);
                        pNodeOfVersion = NULL;
                    }
                }
                // service_address_imaging end

                // service_address_deviceIO begin
                if(NULL != strstr(szNamespace, "deviceIO/wsdl"))
                {
                    strncpy(p_onvif_device->service_address_deviceIO.namesapce, szNamespace, 256);
                    pNodeOfXAddr = findNode(pIXMLDOMNodeTemp, "XAddr");
                    if(NULL != pNodeOfXAddr)
                    {
                        hResult = pNodeOfXAddr->get_text(&bstrXAddr);
                        if(SUCCEEDED(hResult))
                        {
                            szXAddr = _com_util::ConvertBSTRToString(bstrXAddr);
                            SysFreeString(bstrXAddr);
                            bstrXAddr = NULL;
                            strncpy(p_onvif_device->service_address_deviceIO.xaddr, szXAddr, 256);
                            delete[] szXAddr;
                        }
                        do
                        {
                            releaseResult = pNodeOfXAddr->Release();
                        } while(releaseResult > 0);
                        pNodeOfXAddr = NULL;
                    }
                    pNodeOfVersion = findNode(pIXMLDOMNodeTemp, "Version");
                    if(NULL != pNodeOfVersion)
                    {
                        pNodeOfMajor = findNode(pNodeOfVersion, "Major");
                        if(NULL != pNodeOfMajor)
                        {
                            hResult = pNodeOfMajor->get_text(&bstrMajor);
                            if(SUCCEEDED(hResult))
                            {
                                szMajor = _com_util::ConvertBSTRToString(bstrMajor);
                                SysFreeString(bstrMajor);
                                p_onvif_device->service_address_deviceIO.major_version = strtol(szMajor, &end, 10);
                                delete[] szMajor;
                                szMajor = NULL;
                            }
                            do
                            {
                                releaseResult = pNodeOfMajor->Release();
                            } while(releaseResult > 0);
                            pNodeOfMajor = NULL;
                        }
                        pNodeOfMinor = findNode(pNodeOfVersion, "Minor");
                        if(NULL != pNodeOfMinor)
                        {
                            hResult = pNodeOfMinor->get_text(&bstrMinor);
                            if(SUCCEEDED(hResult))
                            {
                                szMinor = _com_util::ConvertBSTRToString(bstrMinor);
                                SysFreeString(bstrMinor);
                                p_onvif_device->service_address_deviceIO.minor_version = strtol(szMinor, &end, 10);
                                delete[] szMinor;
                                szMinor = NULL;
                            }
                            do
                            {
                                releaseResult = pNodeOfMinor->Release();
                            } while(releaseResult > 0);
                            pNodeOfMinor = NULL;
                        }
                        do
                        {
                            releaseResult = pNodeOfVersion->Release();
                        } while(releaseResult > 0);
                        pNodeOfVersion = NULL;
                    }
                }
                // service_address_deviceIO end

                // service_address_analytics begin
                if(NULL != strstr(szNamespace, "analytics/wsdl"))
                {
                    strncpy(p_onvif_device->service_address_analytics.namesapce, szNamespace, 256);
                    pNodeOfXAddr = findNode(pIXMLDOMNodeTemp, "XAddr");
                    if(NULL != pNodeOfXAddr)
                    {
                        hResult = pNodeOfXAddr->get_text(&bstrXAddr);
                        if(SUCCEEDED(hResult))
                        {
                            szXAddr = _com_util::ConvertBSTRToString(bstrXAddr);
                            SysFreeString(bstrXAddr);
                            bstrXAddr = NULL;
                            strncpy(p_onvif_device->service_address_analytics.xaddr, szXAddr, 256);
                            delete[] szXAddr;
                        }
                        do
                        {
                            releaseResult = pNodeOfXAddr->Release();
                        } while(releaseResult > 0);
                        pNodeOfXAddr = NULL;
                    }
                    pNodeOfVersion = findNode(pIXMLDOMNodeTemp, "Version");
                    if(NULL != pNodeOfVersion)
                    {
                        pNodeOfMajor = findNode(pNodeOfVersion, "Major");
                        if(NULL != pNodeOfMajor)
                        {
                            hResult = pNodeOfMajor->get_text(&bstrMajor);
                            if(SUCCEEDED(hResult))
                            {
                                szMajor = _com_util::ConvertBSTRToString(bstrMajor);
                                SysFreeString(bstrMajor);
                                p_onvif_device->service_address_analytics.major_version = strtol(szMajor, &end, 10);
                                delete[] szMajor;
                                szMajor = NULL;
                            }
                            do
                            {
                                releaseResult = pNodeOfMajor->Release();
                            } while(releaseResult > 0);
                            pNodeOfMajor = NULL;
                        }
                        pNodeOfMinor = findNode(pNodeOfVersion, "Minor");
                        if(NULL != pNodeOfMinor)
                        {
                            hResult = pNodeOfMinor->get_text(&bstrMinor);
                            if(SUCCEEDED(hResult))
                            {
                                szMinor = _com_util::ConvertBSTRToString(bstrMinor);
                                SysFreeString(bstrMinor);
                                p_onvif_device->service_address_analytics.minor_version = strtol(szMinor, &end, 10);
                                delete[] szMinor;
                                szMinor = NULL;
                            }
                            do
                            {
                                releaseResult = pNodeOfMinor->Release();
                            } while(releaseResult > 0);
                            pNodeOfMinor = NULL;
                        }
                        do
                        {
                            releaseResult = pNodeOfVersion->Release();
                        } while(releaseResult > 0);
                        pNodeOfVersion = NULL;
                    }
                }
                // service_address_analytics end

                // service_address_recording begin
                if(NULL != strstr(szNamespace, "recording/wsdl"))
                {
                    strncpy(p_onvif_device->service_address_recording.namesapce, szNamespace, 256);
                    pNodeOfXAddr = findNode(pIXMLDOMNodeTemp, "XAddr");
                    if(NULL != pNodeOfXAddr)
                    {
                        hResult = pNodeOfXAddr->get_text(&bstrXAddr);
                        if(SUCCEEDED(hResult))
                        {
                            szXAddr = _com_util::ConvertBSTRToString(bstrXAddr);
                            SysFreeString(bstrXAddr);
                            bstrXAddr = NULL;
                            strncpy(p_onvif_device->service_address_recording.xaddr, szXAddr, 256);
                            delete[] szXAddr;
                        }
                        do
                        {
                            releaseResult = pNodeOfXAddr->Release();
                        } while(releaseResult > 0);
                        pNodeOfXAddr = NULL;
                    }
                    pNodeOfVersion = findNode(pIXMLDOMNodeTemp, "Version");
                    if(NULL != pNodeOfVersion)
                    {
                        pNodeOfMajor = findNode(pNodeOfVersion, "Major");
                        if(NULL != pNodeOfMajor)
                        {
                            hResult = pNodeOfMajor->get_text(&bstrMajor);
                            if(SUCCEEDED(hResult))
                            {
                                szMajor = _com_util::ConvertBSTRToString(bstrMajor);
                                SysFreeString(bstrMajor);
                                p_onvif_device->service_address_recording.major_version = strtol(szMajor, &end, 10);
                                delete[] szMajor;
                                szMajor = NULL;
                            }
                            do
                            {
                                releaseResult = pNodeOfMajor->Release();
                            } while(releaseResult > 0);
                            pNodeOfMajor = NULL;
                        }
                        pNodeOfMinor = findNode(pNodeOfVersion, "Minor");
                        if(NULL != pNodeOfMinor)
                        {
                            hResult = pNodeOfMinor->get_text(&bstrMinor);
                            if(SUCCEEDED(hResult))
                            {
                                szMinor = _com_util::ConvertBSTRToString(bstrMinor);
                                SysFreeString(bstrMinor);
                                p_onvif_device->service_address_recording.minor_version = strtol(szMinor, &end, 10);
                                delete[] szMinor;
                                szMinor = NULL;
                            }
                            do
                            {
                                releaseResult = pNodeOfMinor->Release();
                            } while(releaseResult > 0);
                            pNodeOfMinor = NULL;
                        }
                        do
                        {
                            releaseResult = pNodeOfVersion->Release();
                        } while(releaseResult > 0);
                        pNodeOfVersion = NULL;
                    }
                }
                // service_address_recording end

                // service_address_search_recording begin
                if(NULL != strstr(szNamespace, "search/wsdl"))
                {
                    strncpy(p_onvif_device->service_address_search_recording.namesapce, szNamespace, 256);
                    pNodeOfXAddr = findNode(pIXMLDOMNodeTemp, "XAddr");
                    if(NULL != pNodeOfXAddr)
                    {
                        hResult = pNodeOfXAddr->get_text(&bstrXAddr);
                        if(SUCCEEDED(hResult))
                        {
                            szXAddr = _com_util::ConvertBSTRToString(bstrXAddr);
                            SysFreeString(bstrXAddr);
                            bstrXAddr = NULL;
                            strncpy(p_onvif_device->service_address_search_recording.xaddr, szXAddr, 256);
                            delete[] szXAddr;
                        }
                        do
                        {
                            releaseResult = pNodeOfXAddr->Release();
                        } while(releaseResult > 0);
                        pNodeOfXAddr = NULL;
                    }
                    pNodeOfVersion = findNode(pIXMLDOMNodeTemp, "Version");
                    if(NULL != pNodeOfVersion)
                    {
                        pNodeOfMajor = findNode(pNodeOfVersion, "Major");
                        if(NULL != pNodeOfMajor)
                        {
                            hResult = pNodeOfMajor->get_text(&bstrMajor);
                            if(SUCCEEDED(hResult))
                            {
                                szMajor = _com_util::ConvertBSTRToString(bstrMajor);
                                SysFreeString(bstrMajor);
                                p_onvif_device->service_address_search_recording.major_version = strtol(szMajor, &end, 10);
                                delete[] szMajor;
                                szMajor = NULL;
                            }
                            do
                            {
                                releaseResult = pNodeOfMajor->Release();
                            } while(releaseResult > 0);
                            pNodeOfMajor = NULL;
                        }
                        pNodeOfMinor = findNode(pNodeOfVersion, "Minor");
                        if(NULL != pNodeOfMinor)
                        {
                            hResult = pNodeOfMinor->get_text(&bstrMinor);
                            if(SUCCEEDED(hResult))
                            {
                                szMinor = _com_util::ConvertBSTRToString(bstrMinor);
                                SysFreeString(bstrMinor);
                                p_onvif_device->service_address_search_recording.minor_version = strtol(szMinor, &end, 10);
                                delete[] szMinor;
                                szMinor = NULL;
                            }
                            do
                            {
                                releaseResult = pNodeOfMinor->Release();
                            } while(releaseResult > 0);
                            pNodeOfMinor = NULL;
                        }
                        do
                        {
                            releaseResult = pNodeOfVersion->Release();
                        } while(releaseResult > 0);
                        pNodeOfVersion = NULL;
                    }
                }
                // service_address_search_recording end

                // service_address_replay begin
                if(NULL != strstr(szNamespace, "replay/wsdl"))
                {
                    strncpy(p_onvif_device->service_address_replay.namesapce, szNamespace, 256);
                    pNodeOfXAddr = findNode(pIXMLDOMNodeTemp, "XAddr");
                    if(NULL != pNodeOfXAddr)
                    {
                        hResult = pNodeOfXAddr->get_text(&bstrXAddr);
                        if(SUCCEEDED(hResult))
                        {
                            szXAddr = _com_util::ConvertBSTRToString(bstrXAddr);
                            SysFreeString(bstrXAddr);
                            bstrXAddr = NULL;
                            strncpy(p_onvif_device->service_address_replay.xaddr, szXAddr, 256);
                            delete[] szXAddr;
                        }
                        do
                        {
                            releaseResult = pNodeOfXAddr->Release();
                        } while(releaseResult > 0);
                        pNodeOfXAddr = NULL;
                    }
                    pNodeOfVersion = findNode(pIXMLDOMNodeTemp, "Version");
                    if(NULL != pNodeOfVersion)
                    {
                        pNodeOfMajor = findNode(pNodeOfVersion, "Major");
                        if(NULL != pNodeOfMajor)
                        {
                            hResult = pNodeOfMajor->get_text(&bstrMajor);
                            if(SUCCEEDED(hResult))
                            {
                                szMajor = _com_util::ConvertBSTRToString(bstrMajor);
                                SysFreeString(bstrMajor);
                                p_onvif_device->service_address_replay.major_version = strtol(szMajor, &end, 10);
                                delete[] szMajor;
                                szMajor = NULL;
                            }
                            do
                            {
                                releaseResult = pNodeOfMajor->Release();
                            } while(releaseResult > 0);
                            pNodeOfMajor = NULL;
                        }
                        pNodeOfMinor = findNode(pNodeOfVersion, "Minor");
                        if(NULL != pNodeOfMinor)
                        {
                            hResult = pNodeOfMinor->get_text(&bstrMinor);
                            if(SUCCEEDED(hResult))
                            {
                                szMinor = _com_util::ConvertBSTRToString(bstrMinor);
                                SysFreeString(bstrMinor);
                                p_onvif_device->service_address_replay.minor_version = strtol(szMinor, &end, 10);
                                delete[] szMinor;
                                szMinor = NULL;
                            }
                            do
                            {
                                releaseResult = pNodeOfMinor->Release();
                            } while(releaseResult > 0);
                            pNodeOfMinor = NULL;
                        }
                        do
                        {
                            releaseResult = pNodeOfVersion->Release();
                        } while(releaseResult > 0);
                        pNodeOfVersion = NULL;
                    }
                }
                // service_address_replay end

                // service_address_receiver begin
                if(NULL != strstr(szNamespace, "receiver/wsdl"))
                {
                    strncpy(p_onvif_device->service_address_receiver.namesapce, szNamespace, 256);
                    pNodeOfXAddr = findNode(pIXMLDOMNodeTemp, "XAddr");
                    if(NULL != pNodeOfXAddr)
                    {
                        hResult = pNodeOfXAddr->get_text(&bstrXAddr);
                        if(SUCCEEDED(hResult))
                        {
                            szXAddr = _com_util::ConvertBSTRToString(bstrXAddr);
                            SysFreeString(bstrXAddr);
                            bstrXAddr = NULL;
                            strncpy(p_onvif_device->service_address_receiver.xaddr, szXAddr, 256);
                            delete[] szXAddr;
                        }
                        do
                        {
                            releaseResult = pNodeOfXAddr->Release();
                        } while(releaseResult > 0);
                        pNodeOfXAddr = NULL;
                    }
                    pNodeOfVersion = findNode(pIXMLDOMNodeTemp, "Version");
                    if(NULL != pNodeOfVersion)
                    {
                        pNodeOfMajor = findNode(pNodeOfVersion, "Major");
                        if(NULL != pNodeOfMajor)
                        {
                            hResult = pNodeOfMajor->get_text(&bstrMajor);
                            if(SUCCEEDED(hResult))
                            {
                                szMajor = _com_util::ConvertBSTRToString(bstrMajor);
                                SysFreeString(bstrMajor);
                                p_onvif_device->service_address_receiver.major_version = strtol(szMajor, &end, 10);
                                delete[] szMajor;
                                szMajor = NULL;
                            }
                            do
                            {
                                releaseResult = pNodeOfMajor->Release();
                            } while(releaseResult > 0);
                            pNodeOfMajor = NULL;
                        }
                        pNodeOfMinor = findNode(pNodeOfVersion, "Minor");
                        if(NULL != pNodeOfMinor)
                        {
                            hResult = pNodeOfMinor->get_text(&bstrMinor);
                            if(SUCCEEDED(hResult))
                            {
                                szMinor = _com_util::ConvertBSTRToString(bstrMinor);
                                SysFreeString(bstrMinor);
                                p_onvif_device->service_address_receiver.minor_version = strtol(szMinor, &end, 10);
                                delete[] szMinor;
                                szMinor = NULL;
                            }
                            do
                            {
                                releaseResult = pNodeOfMinor->Release();
                            } while(releaseResult > 0);
                            pNodeOfMinor = NULL;
                        }
                        do
                        {
                            releaseResult = pNodeOfVersion->Release();
                        } while(releaseResult > 0);
                        pNodeOfVersion = NULL;
                    }
                }
                // service_address_receiver end
                delete[] szNamespace;
                szNamespace = NULL;
            }

            do
            {
                releaseResult = pNodeOfNamespace->Release();
            } while(releaseResult > 0);
            pNodeOfNamespace = NULL;
        }

        do
        {
            releaseResult = pIXMLDOMNodeTemp->Release();
        } while(releaseResult > 0);
        pIXMLDOMNodeTemp = NULL;
    }

parseGetServicesResponseCleanUp:

    if(NULL != pIXMLDOMNodeList)
    {
        do
        {
            releaseResult = pIXMLDOMNodeList->Release();
        } while(releaseResult > 0);
        pIXMLDOMNodeList = NULL;
    }

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

    for(size_t i = 0; i < pReceivedDataList->size(); i++)
    {
        if(NULL != (*pReceivedDataList)[i])
        {
            if(NULL != (*pReceivedDataList)[i]->data)
            {
                free((*pReceivedDataList)[i]->data);
            }
            free((*pReceivedDataList)[i]);
        }
    }

    pReceivedDataList->clear();
}