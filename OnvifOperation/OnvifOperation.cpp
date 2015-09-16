// OnvifOperation.cpp : Defines the exported functions for the DLL application.
//
#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include "OnvifOperation.h"
#include "xmlParser.h"

#include "soapH.h"
#include "wsdd.h"
#include "wsseapi.h"
#include "wsaapi.h"

#include "errorHandling.h"

#include <iostream>
#include <string>
#include <vector>
#include <regex>

#include <objbase.h>
#include <rpc.h>
#include <tchar.h>

using namespace std;
// C++ 11

static soap*            pSoap;
//static soap*            SoapForSearch;
static wsdd__ScopesType scopes;
static SOAP_ENV__Header header;

static bool initialsuccess = true;

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

DWORD WINAPI receiveThread(LPVOID lpParam)
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

ONVIFOPERATION_API int init_DLL(void)
{
    pSoap = soap_new1(SOAP_IO_DEFAULT | SOAP_XML_IGNORENS); // ignore namespace, avoid namespace mismatch
    if(NULL == pSoap)
    {
        return -1;
    }

    WSADATA wsaData;
    int result;

    result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if(0 != result)
    {
        handleError(_T("WSAStartup"), _T(__FILE__), __LINE__);
        return -1;
    }

    HRESULT hResult = CoInitialize(NULL);
    if(FAILED(hResult))
    {
        CoUninitialize();
        return -1;
    }

    initialsuccess = true;

    return 0;
}

ONVIFOPERATION_API int uninit_DLL(void)
{
    if(!initialsuccess)
    {
        return -1;
    }

    int result = WSACleanup();
    if(0 != result)
    {
        handleError(_T("WSACleanup"), _T(__FILE__), __LINE__);
        return -1;
    }

    soap_destroy(pSoap);
    soap_end(pSoap);
    soap_done(pSoap);

    pSoap = NULL;

    CoUninitialize();

    initialsuccess = false;

    return 0;
}

ONVIFOPERATION_API onvif_device_list* malloc_device_list(void)
{
    onvif_device_list* p_onvif_device_list_temp = (onvif_device_list*)malloc(sizeof(onvif_device_list));

    if(NULL != p_onvif_device_list_temp)
    {
        p_onvif_device_list_temp->number_of_onvif_devices = 0;
        p_onvif_device_list_temp->p_onvif_devices = (onvif_device*)malloc(1);
    }

    if(NULL == p_onvif_device_list_temp->p_onvif_devices)
    {
        free(p_onvif_device_list_temp);
        p_onvif_device_list_temp = NULL;
    }

    InitializeCriticalSection((LPCRITICAL_SECTION)p_onvif_device_list_temp->critical_section);

    return p_onvif_device_list_temp;
}

ONVIFOPERATION_API void free_device_list(onvif_device_list** pp_onvif_device_list)
{
    if(NULL == pp_onvif_device_list || NULL == (*pp_onvif_device_list))
    {
        return;
    }
    else
    {
        EnterCriticalSection((LPCRITICAL_SECTION)(*pp_onvif_device_list)->critical_section);
        for(size_t i = 0; i < (*pp_onvif_device_list)->number_of_onvif_devices; ++i)
        {
            if(NULL != (*pp_onvif_device_list)->p_onvif_devices[i].p_onvif_ipc_profiles)
            {
                free((*pp_onvif_device_list)->p_onvif_devices[i].p_onvif_ipc_profiles);
            }
        }
        for(size_t i = 0; i < (*pp_onvif_device_list)->number_of_onvif_devices; ++i)
        {
            if(NULL != (*pp_onvif_device_list)->p_onvif_devices[i].p_onvif_NVR_receivers)
            {
                free((*pp_onvif_device_list)->p_onvif_devices[i].p_onvif_NVR_receivers);
            }
        }
        if(NULL != (*pp_onvif_device_list)->p_onvif_devices)
        {
            free((*pp_onvif_device_list)->p_onvif_devices);
        }
        LeaveCriticalSection((LPCRITICAL_SECTION)(*pp_onvif_device_list)->critical_section);
        DeleteCriticalSection((LPCRITICAL_SECTION)(*pp_onvif_device_list)->critical_section);
        free((*pp_onvif_device_list));
        (*pp_onvif_device_list) = NULL;
    }
}

ONVIFOPERATION_API int search_onvif_device(onvif_device_list* p_onvif_device_list, int wait_time)
{
    // check parameters
    if(!initialsuccess || NULL == p_onvif_device_list)
    {
        return -1;
    }

    SOCKET socketForProbe = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(INVALID_SOCKET == socketForProbe)
    {
        handleError(_T("socket"), _T(__FILE__), __LINE__);
        return -1;
    }

    sockaddr_in sockaddrClient;
    memset(&sockaddrClient, 0x0, sizeof(sockaddr_in));
    sockaddrClient.sin_addr.s_addr = htonl(INADDR_ANY);
    sockaddrClient.sin_family = AF_INET;
    sockaddrClient.sin_port = htons(0);

    sockaddr_in sockaddrMulticastAddrForOnvif;
    memset(&sockaddrMulticastAddrForOnvif, 0x0, sizeof(sockaddr_in));
    int result = InetPton(AF_INET, _T("239.255.255.250"), &sockaddrMulticastAddrForOnvif.sin_addr.s_addr);
    if(1 != result)
    {
        handleError(_T("InetPton"), _T(__FILE__), __LINE__);
    }
    sockaddrMulticastAddrForOnvif.sin_family = AF_INET;
    sockaddrMulticastAddrForOnvif.sin_port = htons(3702);

    // for receive thread loop control
    BOOL loop = TRUE;

    DWORD timeOut;
    if(0 > wait_time)
    {
        timeOut = -wait_time;
    }
    else
    {
        timeOut = wait_time * 1000;
    }

    char* pProbeMessage = (char*)malloc(2048);
    if(NULL == pProbeMessage)
    {
        handleError(_T("malloc"), _T(__FILE__), __LINE__);
        closesocket(socketForProbe);
        return -1;
    }

    result = bind(socketForProbe, (struct sockaddr*)&sockaddrClient, sizeof(sockaddr_in));
    if(0 != result)
    {
        handleError(_T("bind"), _T(__FILE__), __LINE__);
        closesocket(socketForProbe);
        return -1;
    }

    result = setsockopt(socketForProbe, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeOut, sizeof(DWORD));
    if(0 != result)
    {
        handleError(_T("setsockopt"), _T(__FILE__), __LINE__);
        closesocket(socketForProbe);
        return -1;
    }

    UUID uuid;
    RPC_STATUS rpcStatus = UuidCreate(&uuid);
    if(RPC_S_OK != rpcStatus)
    {
        handleError(_T("UuidCreate"), _T(__FILE__), __LINE__);
        closesocket(socketForProbe);
        return -1;
    }

    RPC_CSTR RpcCstr;
    rpcStatus = UuidToStringA(&uuid, &RpcCstr);
    if(RPC_S_OK != rpcStatus)
    {
        handleError(_T("UuidToStringA"), _T(__FILE__), __LINE__);
        closesocket(socketForProbe);
        return -1;
    }

    result = _snprintf_s(pProbeMessage, 2048, _TRUNCATE, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n<SOAP-ENV:Envelope xmlns:SOAP-ENV=\"http://www.w3.org/2003/05/soap-envelope\" xmlns:SOAP-ENC=\"http://www.w3.org/2003/05/soap-encoding\" xmlns:wsa=\"http://schemas.xmlsoap.org/ws/2004/08/addressing\" xmlns:wsdd=\"http://schemas.xmlsoap.org/ws/2005/04/discovery\"><SOAP-ENV:Header><wsa:MessageID>urn:uuid:%s</wsa:MessageID><wsa:To SOAP-ENV:mustUnderstand=\"true\">urn:schemas-xmlsoap-org:ws:2005:04:discovery</wsa:To><wsa:Action SOAP-ENV:mustUnderstand=\"true\">http://schemas.xmlsoap.org/ws/2005/04/discovery/Probe</wsa:Action></SOAP-ENV:Header><SOAP-ENV:Body><wsdd:Probe><wsdd:Types></wsdd:Types><wsdd:Scopes></wsdd:Scopes></wsdd:Probe></SOAP-ENV:Body></SOAP-ENV:Envelope>", RpcCstr);
    if(-1 == result)
    {
        handleError(_T("_snprintf_s"), _T(__FILE__), __LINE__);
        closesocket(socketForProbe);
        return -1;
    }

    rpcStatus = RpcStringFreeA(&RpcCstr);
    if(RPC_S_OK != rpcStatus)
    {
        handleError(_T("RpcStringFreeA"), _T(__FILE__), __LINE__);
        closesocket(socketForProbe);
        return -1;
    }

    vector<receivedData*> receivedDataList;
    DWORD threadID;
    receiveThreadParameter parameter;
    parameter.bLoop = &loop;
    parameter.receivedDataList = &receivedDataList;
    parameter.socketForProbe = &socketForProbe;

    HANDLE hThread = CreateThread(NULL, 0, receiveThread, &parameter, 0, &threadID);
    if(NULL == hThread)
    {
        handleError(_T("CreateThread"), _T(__FILE__), __LINE__);
        closesocket(socketForProbe);
        return -1;
    }

    result = sendto(socketForProbe, pProbeMessage, result, 0, (sockaddr*)&sockaddrMulticastAddrForOnvif, sizeof(sockaddr_in));
    if(SOCKET_ERROR == result)
    {
        handleError(_T("sendto"), _T(__FILE__), __LINE__);
        closesocket(socketForProbe);
        return -1;
    }

    free(pProbeMessage);
    pProbeMessage = NULL;

    Sleep(timeOut);

    loop = FALSE;

    WaitForMultipleObjects(1, &hThread, TRUE, INFINITE);

    CloseHandle(hThread);

    result = closesocket(socketForProbe);
    if(0 != result)
    {
        handleError(_T("sendto"), _T(__FILE__), __LINE__);
        return -1;
    }

    // set to no delete
    size_t size = receivedDataList.size();
    for(size_t i = 0; i < size; i++)
    {
        receivedDataList[i]->forDelete = FALSE;
    }

    // set dup to delete
    for(size_t i = 0; i < size; i++)
    {
        for(size_t j = i + 1; j < size; j++)
        {
            if(receivedDataList[i]->endPointAddr.S_un.S_addr == receivedDataList[j]->endPointAddr.S_un.S_addr)
            {
                receivedDataList[j]->forDelete = TRUE;
            }
        }
    }

    // delete
    vector<receivedData*>::iterator it;
    for(size_t i = 0; i < size; i++)
    {
        for(it = receivedDataList.begin(); it != receivedDataList.end(); ++it)
        {
            if((*it)->forDelete)
            {
                if(NULL != (*it)->data)
                {
                    free((*it)->data);
                }
                free((*it));
                receivedDataList.erase(it);
                break;
            }
        }
    }

    EnterCriticalSection((LPCRITICAL_SECTION)p_onvif_device_list->critical_section);

    parseDiscoveredDeviceXML(p_onvif_device_list, &receivedDataList);

    LeaveCriticalSection((LPCRITICAL_SECTION)p_onvif_device_list->critical_section);

    size = receivedDataList.size();

    for(size_t i = 0; i < size; i++)
    {
        if(NULL != receivedDataList[i]->data)
        {
            free(receivedDataList[i]->data);
        }
        free(receivedDataList[i]);
    }

    receivedDataList.clear();

    return 0;
}

ONVIFOPERATION_API int add_onvif_device_manually(onvif_device_list* p_onvif_device_list, char* IP)
{
    onvif_device* p_onvif_device_temp = NULL;
    char* buffer = new char[256];
    if(NULL == buffer)
    {
        return -1;
    }

    if(!initialsuccess || NULL == p_onvif_device_list || NULL == IP)
    {
        delete buffer;
        return -1;
    }

    sprintf(buffer, "http://%s/onvif/device_service", IP);

    EnterCriticalSection((LPCRITICAL_SECTION)p_onvif_device_list->critical_section);

    p_onvif_device_list->number_of_onvif_devices += 1;
    p_onvif_device_temp = (onvif_device*)realloc(p_onvif_device_list->p_onvif_devices, p_onvif_device_list->number_of_onvif_devices * sizeof(onvif_device));
    if(NULL == p_onvif_device_temp)
    {
        delete buffer;
        LeaveCriticalSection((LPCRITICAL_SECTION)p_onvif_device_list->critical_section);
        return -1;
    }
    else
    {
        p_onvif_device_list->p_onvif_devices = p_onvif_device_temp;
        memset(&p_onvif_device_list->p_onvif_devices[p_onvif_device_list->number_of_onvif_devices - 1], 0x0, sizeof(onvif_device));
        strncpy(
            p_onvif_device_list->p_onvif_devices[p_onvif_device_list->number_of_onvif_devices - 1].service_address_device_service.xaddr,
            buffer,
            256);
        strncpy(
            p_onvif_device_list->p_onvif_devices[p_onvif_device_list->number_of_onvif_devices - 1].IPv4,
            IP,
            17);
    }

    delete buffer;
    LeaveCriticalSection((LPCRITICAL_SECTION)p_onvif_device_list->critical_section);
    return 0;
}

ONVIFOPERATION_API int set_onvif_device_authorization_information(onvif_device_list* p_onvif_device_list, char* IP, size_t index, char* username, char* password)
{
    size_t i;

    if(!initialsuccess || NULL == p_onvif_device_list)
    {
        return -1;
    }

    EnterCriticalSection((LPCRITICAL_SECTION)p_onvif_device_list->critical_section);

    if(NULL != IP)
    {
        if(17 < strnlen(IP, 17))
        {
            LeaveCriticalSection((LPCRITICAL_SECTION)p_onvif_device_list->critical_section);
            return -1;
        }
        for(i = 0; i < p_onvif_device_list->number_of_onvif_devices; i++)
        {
            if(0 == strncmp(IP, p_onvif_device_list->p_onvif_devices[i].IPv4, 17))
            {
                index = i;
            }
        }
    }

    if(p_onvif_device_list->number_of_onvif_devices <= index)
    {
        LeaveCriticalSection((LPCRITICAL_SECTION)p_onvif_device_list->critical_section);
        return -1;
    }

    strncpy(
        p_onvif_device_list->p_onvif_devices[index].username,
        username,
        50);

    strncpy(
        p_onvif_device_list->p_onvif_devices[index].password,
        password,
        50);

    LeaveCriticalSection((LPCRITICAL_SECTION)p_onvif_device_list->critical_section);

    return 0;
}

ONVIFOPERATION_API int get_onvif_device_information(onvif_device_list* p_onvif_device_list, char* IP, size_t index)
{
    _tds__GetDeviceInformation          tds__GetDeviceInformation;
    _tds__GetDeviceInformationResponse  tds__GetDeviceInformationResponse;
    _tds__GetNetworkInterfaces          tds__GetNetworkInterfaces;
    _tds__GetNetworkInterfacesResponse  tds__GetNetworkInterfacesResponse;
    size_t i;

    if(!initialsuccess || NULL == p_onvif_device_list)
    {
        return -1;
    }

    EnterCriticalSection((LPCRITICAL_SECTION)p_onvif_device_list->critical_section);

    if(NULL != IP)
    {
        if(17 < strnlen(IP, 17))
        {
            LeaveCriticalSection((LPCRITICAL_SECTION)p_onvif_device_list->critical_section);
            return -1;
        }
        for(i = 0; i < p_onvif_device_list->number_of_onvif_devices; i++)
        {
            if(0 == strncmp(IP, p_onvif_device_list->p_onvif_devices[i].IPv4, 17))
            {
                index = i;
            }
        }
    }

    if(p_onvif_device_list->number_of_onvif_devices <= index)
    {
        LeaveCriticalSection((LPCRITICAL_SECTION)p_onvif_device_list->critical_section);
        return -1;
    }

    soap_set_namespaces(pSoap, device_namespace);

    soap_wsse_add_UsernameTokenDigest(
        pSoap,
        "user",
        p_onvif_device_list->p_onvif_devices[index].username,
        p_onvif_device_list->p_onvif_devices[index].password);

    if(SOAP_OK != soap_call___tds__GetDeviceInformation(pSoap, p_onvif_device_list->p_onvif_devices[index].service_address_device_service.xaddr, NULL, &tds__GetDeviceInformation, &tds__GetDeviceInformationResponse))
    {
        LeaveCriticalSection((LPCRITICAL_SECTION)p_onvif_device_list->critical_section);
        return -1;
    }

    strncpy(
        p_onvif_device_list->p_onvif_devices[index].device_information.firmware_version,
        tds__GetDeviceInformationResponse.FirmwareVersion,
        50);
    strncpy(
        p_onvif_device_list->p_onvif_devices[index].device_information.hardware_Id,
        tds__GetDeviceInformationResponse.HardwareId,
        10);
    strncpy(
        p_onvif_device_list->p_onvif_devices[index].device_information.manufacturer,
        tds__GetDeviceInformationResponse.Manufacturer,
        50);
    strncpy(
        p_onvif_device_list->p_onvif_devices[index].device_information.model,
        tds__GetDeviceInformationResponse.Model,
        50);
    strncpy(
        p_onvif_device_list->p_onvif_devices[index].device_information.serial_number,
        tds__GetDeviceInformationResponse.SerialNumber,
        50);


    soap_set_namespaces(pSoap, device_namespace);

    soap_wsse_add_UsernameTokenDigest(
        pSoap,
        "user",
        p_onvif_device_list->p_onvif_devices[index].username,
        p_onvif_device_list->p_onvif_devices[index].password);

    if(SOAP_OK != soap_call___tds__GetNetworkInterfaces(pSoap, p_onvif_device_list->p_onvif_devices[index].service_address_device_service.xaddr, NULL, &tds__GetNetworkInterfaces, &tds__GetNetworkInterfacesResponse))
    {
        LeaveCriticalSection((LPCRITICAL_SECTION)p_onvif_device_list->critical_section);
        return -1;
    }

    strncpy(
        p_onvif_device_list->p_onvif_devices[index].device_information.MAC_address,
        tds__GetNetworkInterfacesResponse.NetworkInterfaces[0].Info->HwAddress,
        50);

    LeaveCriticalSection((LPCRITICAL_SECTION)p_onvif_device_list->critical_section);

    return 0;
}

ONVIFOPERATION_API int get_onvif_device_service_addresses(onvif_device_list* p_onvif_device_list, char* IP, size_t index)
{
    size_t                      i;
    _tds__GetServices           tds__GetServices;
    _tds__GetServicesResponse   tds__GetServicesResponse;

    if(!initialsuccess || NULL == p_onvif_device_list)
    {
        return -1;
    }

    EnterCriticalSection((LPCRITICAL_SECTION)p_onvif_device_list->critical_section);

    if(NULL != IP)
    {
        if(17 < strnlen(IP, 17))
        {
            LeaveCriticalSection((LPCRITICAL_SECTION)p_onvif_device_list->critical_section);
            return -1;
        }
        for(i = 0; i < p_onvif_device_list->number_of_onvif_devices; i++)
        {
            if(0 == strncmp(IP, p_onvif_device_list->p_onvif_devices[i].IPv4, 17))
            {
                index = i;
            }
        }
    }

    if(p_onvif_device_list->number_of_onvif_devices <= index)
    {
        LeaveCriticalSection((LPCRITICAL_SECTION)p_onvif_device_list->critical_section);
        return -1;
    }

    tds__GetServices.IncludeCapability = xsd__boolean__false_;

    soap_set_namespaces(pSoap, device_namespace);

    soap_wsse_add_UsernameTokenDigest(
        pSoap,
        "user",
        p_onvif_device_list->p_onvif_devices[index].username,
        p_onvif_device_list->p_onvif_devices[index].password);

    if(SOAP_OK != soap_call___tds__GetServices(pSoap, p_onvif_device_list->p_onvif_devices[index].service_address_device_service.xaddr, NULL, &tds__GetServices, &tds__GetServicesResponse))
    {
        LeaveCriticalSection((LPCRITICAL_SECTION)p_onvif_device_list->critical_section);
        return -1;
    }

    for(i = 0; i < tds__GetServicesResponse.__sizeService; i++)
    {
        if(NULL == tds__GetServicesResponse.Service[i].Version)
        {
            continue;
        }

        if(0 == strncmp(tds__GetServicesResponse.Service[i].Namespace, "http://www.onvif.org/ver10/device/wsdl", 256))
        {
            p_onvif_device_list->p_onvif_devices[index].service_address_device_service.major_version = tds__GetServicesResponse.Service[i].Version->Major;
            p_onvif_device_list->p_onvif_devices[index].service_address_device_service.minor_version = tds__GetServicesResponse.Service[i].Version->Minor;
            strncpy(
                p_onvif_device_list->p_onvif_devices[index].service_address_device_service.namesapce,
                tds__GetServicesResponse.Service[i].Namespace,
                256);
            strncpy(
                p_onvif_device_list->p_onvif_devices[index].service_address_device_service.xaddr,
                tds__GetServicesResponse.Service[i].XAddr,
                256);
        }

        if(0 == strncmp(tds__GetServicesResponse.Service[i].Namespace, "http://www.onvif.org/ver10/media/wsdl", 256))
        {
            p_onvif_device_list->p_onvif_devices[index].service_address_media.major_version = tds__GetServicesResponse.Service[i].Version->Major;
            p_onvif_device_list->p_onvif_devices[index].service_address_media.minor_version = tds__GetServicesResponse.Service[i].Version->Minor;
            strncpy(
                p_onvif_device_list->p_onvif_devices[index].service_address_media.namesapce,
                tds__GetServicesResponse.Service[i].Namespace,
                256);
            strncpy(
                p_onvif_device_list->p_onvif_devices[index].service_address_media.xaddr,
                tds__GetServicesResponse.Service[i].XAddr,
                256);
        }

        if(0 == strncmp(tds__GetServicesResponse.Service[i].Namespace, "http://www.onvif.org/ver10/events/wsdl", 256))
        {
            p_onvif_device_list->p_onvif_devices[index].service_address_events.major_version = tds__GetServicesResponse.Service[i].Version->Major;
            p_onvif_device_list->p_onvif_devices[index].service_address_events.minor_version = tds__GetServicesResponse.Service[i].Version->Minor;
            strncpy(
                p_onvif_device_list->p_onvif_devices[index].service_address_events.namesapce,
                tds__GetServicesResponse.Service[i].Namespace,
                256);
            strncpy(
                p_onvif_device_list->p_onvif_devices[index].service_address_events.xaddr,
                tds__GetServicesResponse.Service[i].XAddr,
                256);
        }

        if(0 == strncmp(tds__GetServicesResponse.Service[i].Namespace, "http://www.onvif.org/ver20/imaging/wsdl", 256))
        {
            p_onvif_device_list->p_onvif_devices[index].service_address_imaging.major_version = tds__GetServicesResponse.Service[i].Version->Major;
            p_onvif_device_list->p_onvif_devices[index].service_address_imaging.minor_version = tds__GetServicesResponse.Service[i].Version->Minor;
            strncpy(
                p_onvif_device_list->p_onvif_devices[index].service_address_imaging.namesapce,
                tds__GetServicesResponse.Service[i].Namespace,
                256);
            strncpy(
                p_onvif_device_list->p_onvif_devices[index].service_address_imaging.xaddr,
                tds__GetServicesResponse.Service[i].XAddr,
                256);
        }

        if(0 == strncmp(tds__GetServicesResponse.Service[i].Namespace, "http://www.onvif.org/ver10/deviceIO/wsdl", 256))
        {
            p_onvif_device_list->p_onvif_devices[index].service_address_deviceIO.major_version = tds__GetServicesResponse.Service[i].Version->Major;
            p_onvif_device_list->p_onvif_devices[index].service_address_deviceIO.minor_version = tds__GetServicesResponse.Service[i].Version->Minor;
            strncpy(
                p_onvif_device_list->p_onvif_devices[index].service_address_deviceIO.namesapce,
                tds__GetServicesResponse.Service[i].Namespace,
                256);
            strncpy(
                p_onvif_device_list->p_onvif_devices[index].service_address_deviceIO.xaddr,
                tds__GetServicesResponse.Service[i].XAddr,
                256);
        }

        if(0 == strncmp(tds__GetServicesResponse.Service[i].Namespace, "http://www.onvif.org/ver20/analytics/wsdl", 256))
        {
            p_onvif_device_list->p_onvif_devices[index].service_address_analytics.major_version = tds__GetServicesResponse.Service[i].Version->Major;
            p_onvif_device_list->p_onvif_devices[index].service_address_analytics.minor_version = tds__GetServicesResponse.Service[i].Version->Minor;
            strncpy(
                p_onvif_device_list->p_onvif_devices[index].service_address_analytics.namesapce,
                tds__GetServicesResponse.Service[i].Namespace,
                256);
            strncpy(
                p_onvif_device_list->p_onvif_devices[index].service_address_analytics.xaddr,
                tds__GetServicesResponse.Service[i].XAddr,
                256);
        }

        if(0 == strncmp(tds__GetServicesResponse.Service[i].Namespace, "http://www.onvif.org/ver10/recording/wsdl", 256))
        {
            p_onvif_device_list->p_onvif_devices[index].service_address_recording.major_version = tds__GetServicesResponse.Service[i].Version->Major;
            p_onvif_device_list->p_onvif_devices[index].service_address_recording.minor_version = tds__GetServicesResponse.Service[i].Version->Minor;
            strncpy(
                p_onvif_device_list->p_onvif_devices[index].service_address_recording.namesapce,
                tds__GetServicesResponse.Service[i].Namespace,
                256);
            strncpy(
                p_onvif_device_list->p_onvif_devices[index].service_address_recording.xaddr,
                tds__GetServicesResponse.Service[i].XAddr,
                256);
        }

        if(0 == strncmp(tds__GetServicesResponse.Service[i].Namespace, "http://www.onvif.org/ver10/search/wsdl", 256))
        {
            p_onvif_device_list->p_onvif_devices[index].service_address_search_recording.major_version = tds__GetServicesResponse.Service[i].Version->Major;
            p_onvif_device_list->p_onvif_devices[index].service_address_search_recording.minor_version = tds__GetServicesResponse.Service[i].Version->Minor;
            strncpy(
                p_onvif_device_list->p_onvif_devices[index].service_address_search_recording.namesapce,
                tds__GetServicesResponse.Service[i].Namespace,
                256);
            strncpy(
                p_onvif_device_list->p_onvif_devices[index].service_address_search_recording.xaddr,
                tds__GetServicesResponse.Service[i].XAddr,
                256);
        }

        if(0 == strncmp(tds__GetServicesResponse.Service[i].Namespace, "http://www.onvif.org/ver10/replay/wsdl", 256))
        {
            p_onvif_device_list->p_onvif_devices[index].service_address_replay.major_version = tds__GetServicesResponse.Service[i].Version->Major;
            p_onvif_device_list->p_onvif_devices[index].service_address_replay.minor_version = tds__GetServicesResponse.Service[i].Version->Minor;
            strncpy(
                p_onvif_device_list->p_onvif_devices[index].service_address_replay.namesapce,
                tds__GetServicesResponse.Service[i].Namespace,
                256);
            strncpy(
                p_onvif_device_list->p_onvif_devices[index].service_address_replay.xaddr,
                tds__GetServicesResponse.Service[i].XAddr,
                256);
        }

        if(0 == strncmp(tds__GetServicesResponse.Service[i].Namespace, "http://www.onvif.org/ver10/receiver/wsdl", 256))
        {
            p_onvif_device_list->p_onvif_devices[index].service_address_receiver.major_version = tds__GetServicesResponse.Service[i].Version->Major;
            p_onvif_device_list->p_onvif_devices[index].service_address_receiver.minor_version = tds__GetServicesResponse.Service[i].Version->Minor;
            strncpy(
                p_onvif_device_list->p_onvif_devices[index].service_address_receiver.namesapce,
                tds__GetServicesResponse.Service[i].Namespace,
                256);
            strncpy(
                p_onvif_device_list->p_onvif_devices[index].service_address_receiver.xaddr,
                tds__GetServicesResponse.Service[i].XAddr,
                256);
        }
    }

    LeaveCriticalSection((LPCRITICAL_SECTION)p_onvif_device_list->critical_section);

    return 0;
}

ONVIFOPERATION_API int get_onvif_ipc_profiles(onvif_device_list* p_onvif_device_list, char* IP, size_t index)
{
    size_t                      i;
    _trt__GetProfiles           getProfiles;
    _trt__GetProfilesResponse   getProfilesResponse;
    _trt__GetStreamUri          getStreamUri;
    _trt__GetStreamUriResponse  getStreamUriResponse;

    if(!initialsuccess || NULL == p_onvif_device_list)
    {
        return -1;
    }

    EnterCriticalSection((LPCRITICAL_SECTION)p_onvif_device_list->critical_section);

    if(NULL != IP)
    {
        if(17 < strnlen(IP, 17))
        {
            LeaveCriticalSection((LPCRITICAL_SECTION)p_onvif_device_list->critical_section);
            return -1;
        }
        for(i = 0; i < p_onvif_device_list->number_of_onvif_devices; i++)
        {
            if(0 == strncmp(IP, p_onvif_device_list->p_onvif_devices[i].IPv4, 17))
            {
                index = i;
            }
        }
    }

    if(p_onvif_device_list->number_of_onvif_devices <= index || 17 > strnlen(p_onvif_device_list->p_onvif_devices[index].service_address_media.xaddr, 256))
    {
        LeaveCriticalSection((LPCRITICAL_SECTION)p_onvif_device_list->critical_section);
        return -1;
    }

    getStreamUri.StreamSetup = (tt__StreamSetup*)malloc(sizeof(tt__StreamSetup));
    if(NULL == getStreamUri.StreamSetup)
    {
        LeaveCriticalSection((LPCRITICAL_SECTION)p_onvif_device_list->critical_section);
        return -1;
    }

    getStreamUri.StreamSetup->Transport = (tt__Transport*)malloc(sizeof(tt__Transport));
    if(NULL == getStreamUri.StreamSetup->Transport)
    {
        free(getStreamUri.StreamSetup);
        LeaveCriticalSection((LPCRITICAL_SECTION)p_onvif_device_list->critical_section);
        return -1;
    }

    getStreamUri.StreamSetup->Transport->Tunnel = (tt__Transport*)malloc(sizeof(tt__Transport));
    if(NULL == getStreamUri.StreamSetup->Transport->Tunnel)
    {
        free(getStreamUri.StreamSetup->Transport);
        free(getStreamUri.StreamSetup);
        LeaveCriticalSection((LPCRITICAL_SECTION)p_onvif_device_list->critical_section);
        return -1;
    }

    getStreamUri.StreamSetup->Stream = tt__StreamType__RTP_Unicast;
    getStreamUri.StreamSetup->Transport->Protocol = tt__TransportProtocol__RTSP;
    getStreamUri.StreamSetup->Transport->Tunnel->Protocol = tt__TransportProtocol__RTSP;
    getStreamUri.StreamSetup->Transport->Tunnel->Tunnel = NULL;
    getStreamUri.StreamSetup->__size = 1;
    getStreamUri.StreamSetup->__any = NULL;
    getStreamUri.StreamSetup->__anyAttribute = NULL;


    soap_set_namespaces(pSoap, media_namespace);

    soap_wsse_add_UsernameTokenDigest(
        pSoap,
        "user",
        p_onvif_device_list->p_onvif_devices[index].username,
        p_onvif_device_list->p_onvif_devices[index].password);

    if(SOAP_OK != soap_call___trt__GetProfiles(pSoap, p_onvif_device_list->p_onvif_devices[index].service_address_media.xaddr, NULL, &getProfiles, &getProfilesResponse))
    {
        LeaveCriticalSection((LPCRITICAL_SECTION)p_onvif_device_list->critical_section);
        free(getStreamUri.StreamSetup->Transport->Tunnel);
        free(getStreamUri.StreamSetup->Transport);
        free(getStreamUri.StreamSetup);
        return -1;
    }

    // free preverious onvif device profiles list
    if(NULL != p_onvif_device_list->p_onvif_devices[index].p_onvif_ipc_profiles)
    {
        free(p_onvif_device_list->p_onvif_devices[index].p_onvif_ipc_profiles);
    }

    p_onvif_device_list->p_onvif_devices[index].number_of_onvif_ipc_profiles = getProfilesResponse.__sizeProfiles;
    p_onvif_device_list->p_onvif_devices[index].p_onvif_ipc_profiles = (onvif_ipc_profile*)malloc(getProfilesResponse.__sizeProfiles * sizeof(onvif_ipc_profile));
    if(NULL == p_onvif_device_list->p_onvif_devices[index].p_onvif_ipc_profiles)
    {
        LeaveCriticalSection((LPCRITICAL_SECTION)p_onvif_device_list->critical_section);
        free(getStreamUri.StreamSetup->Transport->Tunnel);
        free(getStreamUri.StreamSetup->Transport);
        free(getStreamUri.StreamSetup);
        return -1;
    }
    memset(p_onvif_device_list->p_onvif_devices[index].p_onvif_ipc_profiles, 0x0, getProfilesResponse.__sizeProfiles * sizeof(onvif_ipc_profile));

    for(i = 0; i < p_onvif_device_list->p_onvif_devices[index].number_of_onvif_ipc_profiles; ++i)
    {
        if(NULL != getProfilesResponse.Profiles)
        {
            if(NULL != getProfilesResponse.Profiles[i].AudioEncoderConfiguration)
            {
                p_onvif_device_list->p_onvif_devices[index].p_onvif_ipc_profiles[i].AudioEncoderConfiguration.Bitrate =
                    getProfilesResponse.Profiles[i].AudioEncoderConfiguration->Bitrate;

                p_onvif_device_list->p_onvif_devices[index].p_onvif_ipc_profiles[i].AudioEncoderConfiguration.encoding =
                    (audio_encoding)getProfilesResponse.Profiles[i].AudioEncoderConfiguration->Encoding;

                strncpy(
                    p_onvif_device_list->p_onvif_devices[index].p_onvif_ipc_profiles[i].AudioEncoderConfiguration.Name,
                    getProfilesResponse.Profiles[i].AudioEncoderConfiguration->Name,
                    30);

                p_onvif_device_list->p_onvif_devices[index].p_onvif_ipc_profiles[i].AudioEncoderConfiguration.SampleRate =
                    getProfilesResponse.Profiles[i].AudioEncoderConfiguration->SampleRate;

                p_onvif_device_list->p_onvif_devices[index].p_onvif_ipc_profiles[i].AudioEncoderConfiguration.UseCount =
                    getProfilesResponse.Profiles[i].AudioEncoderConfiguration->UseCount;
            }

            if(NULL != getProfilesResponse.Profiles[i].AudioSourceConfiguration)
            {
                strncpy(
                    p_onvif_device_list->p_onvif_devices[index].p_onvif_ipc_profiles[i].AudioSourceConfiguration.Name,
                    getProfilesResponse.Profiles[i].AudioSourceConfiguration->Name,
                    30);

                strncpy(
                    p_onvif_device_list->p_onvif_devices[index].p_onvif_ipc_profiles[i].AudioSourceConfiguration.SourceToken,
                    getProfilesResponse.Profiles[i].AudioSourceConfiguration->SourceToken,
                    30);

                p_onvif_device_list->p_onvif_devices[index].p_onvif_ipc_profiles[i].AudioSourceConfiguration.UseCount =
                    getProfilesResponse.Profiles[i].AudioSourceConfiguration->UseCount;
            }

            getStreamUri.ProfileToken = getProfilesResponse.Profiles[i].token;

            soap_set_namespaces(pSoap, media_namespace);

            soap_wsse_add_UsernameTokenDigest(
                pSoap,
                "user",
                p_onvif_device_list->p_onvif_devices[index].username,
                p_onvif_device_list->p_onvif_devices[index].password);

            soap_call___trt__GetStreamUri(
                pSoap,
                p_onvif_device_list->p_onvif_devices[index].service_address_media.xaddr,
                NULL,
                &getStreamUri,
                &getStreamUriResponse);

            if(NULL != getStreamUriResponse.MediaUri)
            {
                p_onvif_device_list->p_onvif_devices[index].p_onvif_ipc_profiles[i].MediaUri.InvalidAfterConnect =
                    getStreamUriResponse.MediaUri->InvalidAfterConnect;

                p_onvif_device_list->p_onvif_devices[index].p_onvif_ipc_profiles[i].MediaUri.InvalidAfterReboot =
                    getStreamUriResponse.MediaUri->InvalidAfterReboot;

                p_onvif_device_list->p_onvif_devices[index].p_onvif_ipc_profiles[i].MediaUri.Timeout =
                    getStreamUriResponse.MediaUri->Timeout;

                strncpy(
                    p_onvif_device_list->p_onvif_devices[index].p_onvif_ipc_profiles[i].MediaUri.URI,
                    getStreamUriResponse.MediaUri->Uri,
                    256);
            }

            strncpy(
                p_onvif_device_list->p_onvif_devices[index].p_onvif_ipc_profiles[i].name,
                getProfilesResponse.Profiles[i].Name,
                30);

            strncpy(
                p_onvif_device_list->p_onvif_devices[index].p_onvif_ipc_profiles[i].token,
                getProfilesResponse.Profiles[i].token,
                30);

            if(NULL != getProfilesResponse.Profiles[i].VideoEncoderConfiguration)
            {
                p_onvif_device_list->p_onvif_devices[index].p_onvif_ipc_profiles[i].VideoEncoderConfiguration.encoding =
                    (video_encoding)getProfilesResponse.Profiles[i].VideoEncoderConfiguration->Encoding;

                if(NULL != getProfilesResponse.Profiles[i].VideoEncoderConfiguration->H264)
                {
                    p_onvif_device_list->p_onvif_devices[index].p_onvif_ipc_profiles[i].VideoEncoderConfiguration.H264.GovLength =
                        getProfilesResponse.Profiles[i].VideoEncoderConfiguration->H264->GovLength;

                    p_onvif_device_list->p_onvif_devices[index].p_onvif_ipc_profiles[i].VideoEncoderConfiguration.H264.Profile =
                        (H264Profile)getProfilesResponse.Profiles[i].VideoEncoderConfiguration->H264->H264Profile;
                }

                strncpy(
                    p_onvif_device_list->p_onvif_devices[index].p_onvif_ipc_profiles[i].VideoEncoderConfiguration.Name,
                    getProfilesResponse.Profiles[i].VideoEncoderConfiguration->Name,
                    30);

                p_onvif_device_list->p_onvif_devices[index].p_onvif_ipc_profiles[i].VideoEncoderConfiguration.Quality =
                    getProfilesResponse.Profiles[i].VideoEncoderConfiguration->Quality;

                if(NULL != getProfilesResponse.Profiles[i].VideoEncoderConfiguration->RateControl)
                {
                    p_onvif_device_list->p_onvif_devices[index].p_onvif_ipc_profiles[i].VideoEncoderConfiguration.RateControl.BitrateLimit =
                        getProfilesResponse.Profiles[i].VideoEncoderConfiguration->RateControl->BitrateLimit;

                    p_onvif_device_list->p_onvif_devices[index].p_onvif_ipc_profiles[i].VideoEncoderConfiguration.RateControl.EncodingInterval =
                        getProfilesResponse.Profiles[i].VideoEncoderConfiguration->RateControl->EncodingInterval;

                    p_onvif_device_list->p_onvif_devices[index].p_onvif_ipc_profiles[i].VideoEncoderConfiguration.RateControl.FrameRateLimit =
                       getProfilesResponse.Profiles[i].VideoEncoderConfiguration->RateControl->FrameRateLimit;
                }

                if(NULL != getProfilesResponse.Profiles[i].VideoEncoderConfiguration->Resolution)
                {
                    p_onvif_device_list->p_onvif_devices[index].p_onvif_ipc_profiles[i].VideoEncoderConfiguration.Resolution.Height =
                        getProfilesResponse.Profiles[i].VideoEncoderConfiguration->Resolution->Height;

                    p_onvif_device_list->p_onvif_devices[index].p_onvif_ipc_profiles[i].VideoEncoderConfiguration.Resolution.Width =
                        getProfilesResponse.Profiles[i].VideoEncoderConfiguration->Resolution->Width;
                }

                p_onvif_device_list->p_onvif_devices[index].p_onvif_ipc_profiles[i].VideoEncoderConfiguration.UseCount =
                    getProfilesResponse.Profiles[i].VideoEncoderConfiguration->UseCount;
            }

            if(NULL != getProfilesResponse.Profiles[i].VideoSourceConfiguration)
            {
                strncpy(
                    p_onvif_device_list->p_onvif_devices[index].p_onvif_ipc_profiles[i].VideoSourceConfiguration.Name,
                    getProfilesResponse.Profiles[i].VideoSourceConfiguration->Name,
                    30);

                strncpy(
                    p_onvif_device_list->p_onvif_devices[index].p_onvif_ipc_profiles[i].VideoSourceConfiguration.SourceToken,
                    getProfilesResponse.Profiles[i].VideoSourceConfiguration->token,
                    30);

                p_onvif_device_list->p_onvif_devices[index].p_onvif_ipc_profiles[i].VideoSourceConfiguration.UseCount =
                    getProfilesResponse.Profiles[i].VideoSourceConfiguration->UseCount;
            }
        }
    }

    LeaveCriticalSection((LPCRITICAL_SECTION)p_onvif_device_list->critical_section);

    free(getStreamUri.StreamSetup->Transport->Tunnel);
    free(getStreamUri.StreamSetup->Transport);
    free(getStreamUri.StreamSetup);

    return 0;
}

ONVIFOPERATION_API int get_onvif_nvr_receivers(onvif_device_list* p_onvif_device_list, char* IP, size_t index)
{
    size_t                      i;
    _trv__GetReceivers          GetReceivers;
    _trv__GetReceiversResponse  GetReceiversResponse;

    if(!initialsuccess || NULL == p_onvif_device_list)
    {
        return -1;
    }

    EnterCriticalSection((LPCRITICAL_SECTION)p_onvif_device_list->critical_section);

    if(NULL != IP)
    {
        if(17 < strnlen(IP, 17))
        {
            LeaveCriticalSection((LPCRITICAL_SECTION)p_onvif_device_list->critical_section);
            return -1;
        }
        for(i = 0; i < p_onvif_device_list->number_of_onvif_devices; i++)
        {
            if(0 == strncmp(IP, p_onvif_device_list->p_onvif_devices[i].IPv4, 17))
            {
                index = i;
            }
        }
    }

    if(p_onvif_device_list->number_of_onvif_devices <= index || 17 > strnlen(p_onvif_device_list->p_onvif_devices[index].service_address_receiver.xaddr, 256))
    {
        LeaveCriticalSection((LPCRITICAL_SECTION)p_onvif_device_list->critical_section);
        return -1;
    }

    soap_set_namespaces(pSoap, receiver_namespace);

    soap_wsse_add_UsernameTokenDigest(
        pSoap,
        "user",
        p_onvif_device_list->p_onvif_devices[index].username,
        p_onvif_device_list->p_onvif_devices[index].password);


    if(SOAP_OK != soap_call___trv__GetReceivers(pSoap, p_onvif_device_list->p_onvif_devices[index].service_address_receiver.xaddr, NULL, &GetReceivers, &GetReceiversResponse))
    {
        LeaveCriticalSection((LPCRITICAL_SECTION)p_onvif_device_list->critical_section);
        return -1;
    }

    // free preverious onvif NVR receivers list
    if(NULL != p_onvif_device_list->p_onvif_devices[index].p_onvif_NVR_receivers)
    {
        free(p_onvif_device_list->p_onvif_devices[index].p_onvif_NVR_receivers);
    }

    p_onvif_device_list->p_onvif_devices[index].number_of_onvif_NVR_receivers = GetReceiversResponse.__sizeReceivers;
    p_onvif_device_list->p_onvif_devices[index].p_onvif_NVR_receivers = (onvif_NVR_receiver*)malloc(p_onvif_device_list->p_onvif_devices[index].number_of_onvif_NVR_receivers * sizeof(onvif_NVR_receiver));
    if(NULL == p_onvif_device_list->p_onvif_devices[index].p_onvif_NVR_receivers)
    {
        LeaveCriticalSection((LPCRITICAL_SECTION)p_onvif_device_list->critical_section);
        return -1;
    }

    for(i = 0; i < p_onvif_device_list->p_onvif_devices[index].number_of_onvif_NVR_receivers; i++)
    {
        if(NULL == GetReceiversResponse.Receivers)
        {
            continue;
        }

        strncpy(
            p_onvif_device_list->p_onvif_devices[index].p_onvif_NVR_receivers[i].token,
            GetReceiversResponse.Receivers[i].Token,
            30);

        if(NULL != GetReceiversResponse.Receivers[i].Configuration)
        {
            strncpy(
                p_onvif_device_list->p_onvif_devices[index].p_onvif_NVR_receivers[i].configuration.media_URI,
                GetReceiversResponse.Receivers[i].Configuration->MediaUri,
                256);

            p_onvif_device_list->p_onvif_devices[index].p_onvif_NVR_receivers[i].configuration.mode =
                (ReceiverMode)GetReceiversResponse.Receivers[i].Configuration->Mode;

            if(NULL != GetReceiversResponse.Receivers[i].Configuration->StreamSetup)
            {
                if(NULL != GetReceiversResponse.Receivers[i].Configuration->StreamSetup->Transport)
                {
                    p_onvif_device_list->p_onvif_devices[index].p_onvif_NVR_receivers[i].configuration.stream_setup.protocol =
                        (TransportProtocol)GetReceiversResponse.Receivers[i].Configuration->StreamSetup->Transport->Protocol;
                }
                p_onvif_device_list->p_onvif_devices[index].p_onvif_NVR_receivers[i].configuration.stream_setup.stream =
                    (StreamType)GetReceiversResponse.Receivers[i].Configuration->StreamSetup->Stream;
            }
        }
    }

    LeaveCriticalSection((LPCRITICAL_SECTION)p_onvif_device_list->critical_section);

    return 0;
}