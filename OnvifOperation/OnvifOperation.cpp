// OnvifOperation.cpp : Defines the exported functions for the DLL application.
//

#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include "OnvifOperation.h"

#include "soapH.h"
#include "wsdd.h"
#include "wsseapi.h"
#include "wsaapi.h"

#include <iostream>
#include <string>
#include <vector>
#include <regex>
using namespace std;
// C++ 11

typedef struct Container
{
    __wsdd__ProbeMatches          probeMatches;
    _tds__GetCapabilitiesResponse getCapabilitiesResponse;
    _trt__GetProfilesResponse     getProfilesResponse;
    _trt__GetStreamUriResponse    getStreamUriResponse;
    BOOL                          duplicated;
}device_info_container;

static soap*                                  pSoap;
static soap*                                  pSoapForSearch;
static wsdd__ScopesType                       scopes;
static SOAP_ENV__Header                       header;

static vector<device_info_container*>           deviceInfoList;
static vector<device_info_container*>::iterator deviceInfoListIterator;

static wsdd__ProbeType                        probe;
static _tds__GetCapabilities                  getCapabilities;
static _trt__GetProfiles                      getProfiles;
static _trt__GetStreamUri                     getStreamUri;

static tt__StreamSetup*                       pTemp1;
static tt__Transport*                         pTemp2;
static tt__Transport*                         pTemp3;

static bool initialsuccess = false;
static bool device_list_locked = false;

int getRTSP(vector<device_info_container*>::iterator& Iterator, char* username, char* password)
{
    if(NULL != (*Iterator)->getStreamUriResponse.MediaUri)
    {
        if(1 < (*Iterator)->getStreamUriResponse.MediaUri->Uri.size())
        {
            return -1;
        }
    }

    // get capabilities 
    if(NULL == (*Iterator)->probeMatches.wsdd__ProbeMatches)
    {
        return -1;
    }
    if(NULL == (*Iterator)->probeMatches.wsdd__ProbeMatches->ProbeMatch)
    {
        return -1;
    }
    if(NULL == (*Iterator)->probeMatches.wsdd__ProbeMatches->ProbeMatch->XAddrs)
    {
        return -1;
    }

    soap_set_namespaces(pSoap, device_namespace);

    soap_wsse_add_UsernameTokenDigest(pSoap, "user", username, password);

    soap_call___tds__GetCapabilities(pSoap, (*Iterator)->probeMatches.wsdd__ProbeMatches->ProbeMatch->XAddrs, NULL, &getCapabilities, (*Iterator)->getCapabilitiesResponse);

    // get profiles 
    if(NULL == (*Iterator)->getCapabilitiesResponse.Capabilities)
    {
        return -1;
    }
    if(NULL == (*Iterator)->getCapabilitiesResponse.Capabilities->Media)
    {
        return -1;
    }

    soap_set_namespaces(pSoap, getProfilesNamespace);

    soap_wsse_add_UsernameTokenDigest(pSoap, "user", username, password);

    soap_call___trt__GetProfiles(pSoap, (*Iterator)->getCapabilitiesResponse.Capabilities->Media->XAddr.c_str(), NULL, &getProfiles, (*Iterator)->getProfilesResponse);

    // get stream uri
    if(0 == (*Iterator)->getProfilesResponse.Profiles.size())
    {
        return -1;
    }

    getStreamUri.ProfileToken = (*(*Iterator)->getProfilesResponse.Profiles.begin())->token;

    soap_set_namespaces(pSoap, getStreamUriNamespace);

    soap_wsse_add_UsernameTokenDigest(pSoap, "user", username, password);
    soap_call___trt__GetStreamUri(pSoap, (*Iterator)->getCapabilitiesResponse.Capabilities->Media->XAddr.c_str(), NULL, &getStreamUri, (*Iterator)->getStreamUriResponse);

    return 0;
}

ONVIFOPERATION_API int init_DLL(void)
{
    pSoap = soap_new1(SOAP_IO_DEFAULT | SOAP_XML_IGNORENS); // ignore namespace, avoid namespace mismatch
    if(NULL == pSoap)
    {
        return -1;
    }

    pSoapForSearch = soap_new1(SOAP_IO_DEFAULT | SOAP_XML_IGNORENS); // ignore namespace, avoid namespace mismatch
    if(NULL == pSoap)
    {
        return -1;
    }

    getCapabilities.Category.resize(1);
    getCapabilities.Category[0] = tt__CapabilityCategory__Media;

    pTemp1 = new tt__StreamSetup;
    if(NULL == pTemp1)
    {
        return -1;
    }
    getStreamUri.StreamSetup = pTemp1;
    getStreamUri.StreamSetup->Stream = tt__StreamType__RTP_Unicast;

    pTemp2 = new tt__Transport;
    if(NULL == pTemp2)
    {
        return -1;
    }
    getStreamUri.StreamSetup->Transport = pTemp2;
    getStreamUri.StreamSetup->Transport->Protocol = tt__TransportProtocol__UDP;

    pTemp3 = new tt__Transport;
    if(NULL == pTemp3)
    {
        return -1;
    }
    getStreamUri.StreamSetup->Transport->Tunnel = pTemp3;

    getStreamUri.StreamSetup->__any.resize(1);
    getStreamUri.StreamSetup->__any[0] = NULL;
    getStreamUri.StreamSetup->__anyAttribute = NULL;

    initialsuccess = true;

    return 0;
}

ONVIFOPERATION_API int uninit_DLL(void)
{
    if(!initialsuccess)
    {
        return -1;
    }

    delete pTemp3;
    delete pTemp2;
    delete pTemp1;

    pTemp3 = NULL;
    pTemp2 = NULL;
    pTemp1 = NULL;

    soap_destroy(pSoapForSearch);
    soap_end(pSoapForSearch);
    soap_done(pSoapForSearch);

    soap_destroy(pSoap); // remove deserialized class instances (C++ only) 
    soap_end(pSoap); // clean up and remove deserialized data 
    soap_free(pSoap); // detach and free runtime context 

    pSoapForSearch = NULL;
    pSoap = NULL;

    initialsuccess = false;

    return 0;
}

ONVIFOPERATION_API onvif_device_list* malloc_device_list(void)
{
    onvif_device_list* p_onvif_device_list_temp = (onvif_device_list*)malloc(sizeof(onvif_device_list));

    if(NULL != p_onvif_device_list_temp)
    {
        p_onvif_device_list_temp->number_of_onvif_device = 0;
        p_onvif_device_list_temp->devcie_list_lock = false;
        p_onvif_device_list_temp->p_onvif_device = (onvif_device*)malloc(1);
    }
    else if(NULL == p_onvif_device_list_temp->p_onvif_device)
    {
        free(p_onvif_device_list_temp);
        p_onvif_device_list_temp = NULL;
    }

    return p_onvif_device_list_temp;
}

ONVIFOPERATION_API void free_device_list(onvif_device_list** pp_onvif_device_list)
{
    if(NULL == pp_onvif_device_list)
    {
        return;
    }

    if(NULL != (*pp_onvif_device_list)->p_onvif_device)
    {
        free((*pp_onvif_device_list)->p_onvif_device);
    }

    if(NULL != (*pp_onvif_device_list))
    {
        free((*pp_onvif_device_list));
    }

    (*pp_onvif_device_list) = NULL;
}

ONVIFOPERATION_API int search_ONVIF_device(onvif_device_list* p_onvif_device_list, int waitTime)
{
    vector<string>              device_service_address_list;
    vector<string>              device_IPv4_list;
    onvif_device*               p_onvif_device_temp;
    size_t                      i;
    __wsdd__ProbeMatches        probeMatches;
    regex                       expression("\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}");
    smatch                      match;

    if(!initialsuccess || NULL == p_onvif_device_list)
    {
        return -1;
    }

    soap_default_SOAP_ENV__Header(pSoapForSearch, &header);
    soap_set_namespaces(pSoapForSearch, probeNamespace);
    pSoapForSearch->recv_timeout = waitTime;

    header.wsa__MessageID = (char*)soap_wsa_rand_uuid(pSoapForSearch);
    if(NULL == header.wsa__MessageID)
    {
        return -1;
    }

    header.wsa__To = "urn:schemas-xmlsoap-org:ws:2005:04:discovery";
    header.wsa__Action = "http://schemas.xmlsoap.org/ws/2005/04/discovery/Probe";
    pSoapForSearch->header = &header;

    soap_default_wsdd__ScopesType(pSoapForSearch, &scopes);
    scopes.__item = "onvif://www.onvif.org";
    soap_default_wsdd__ProbeType(pSoapForSearch, &probe);
    probe.Scopes = &scopes;
    probe.Types = ""; /*ns1:NetworkVideoTransmitter*/
    //probe.Types = "NetworkVideoTransmitter";

    if(SOAP_OK != soap_send___wsdd__Probe(pSoapForSearch, "soap.udp://239.255.255.250:3702", NULL, &probe))
    {
        return -1;
    }

    // get match result and put into vector
    while(TRUE)
    {
        if(SOAP_OK != soap_recv___wsdd__ProbeMatches(pSoapForSearch, &probeMatches))
        {
            break;
        }
        else
        {
            if(NULL == probeMatches.wsdd__ProbeMatches)
            {
                continue;
            }
            if(NULL == probeMatches.wsdd__ProbeMatches->ProbeMatch)
            {
                continue;
            }
            if(NULL == probeMatches.wsdd__ProbeMatches->ProbeMatch->XAddrs)
            {
                continue;
            }

            device_service_address_list.push_back(probeMatches.wsdd__ProbeMatches->ProbeMatch->XAddrs);
        }
    }

    device_IPv4_list.resize(device_service_address_list.size());
    for(i = 0; i < device_IPv4_list.size(); ++i)
    {
        auto words_begin = sregex_iterator(device_service_address_list[i].begin(), device_service_address_list[i].end(), expression);
        auto words_end = sregex_iterator();

        for(sregex_iterator iterator = words_begin; iterator != words_end; ++iterator)
        {
            match = *iterator;
        }

        device_IPv4_list[i] = match.str();
    }

    while(p_onvif_device_list->devcie_list_lock)
    {
        Sleep(10);
    }
    p_onvif_device_list->devcie_list_lock = true;


    p_onvif_device_list->number_of_onvif_device = device_service_address_list.size();
    p_onvif_device_temp = (onvif_device*)realloc(p_onvif_device_list->p_onvif_device, p_onvif_device_list->number_of_onvif_device * sizeof(onvif_device));
    if(NULL == p_onvif_device_temp)
    {
        p_onvif_device_list->devcie_list_lock = false;
        return -1;
    }
    else
    {
        p_onvif_device_list->p_onvif_device = p_onvif_device_temp;
    }

    for(i = 0; i < p_onvif_device_list->number_of_onvif_device; ++i)
    {
        strncpy(p_onvif_device_list->p_onvif_device[i].device_service_address, device_service_address_list[i].c_str(), 256);
        strncpy(p_onvif_device_list->p_onvif_device[i].IPv4, device_IPv4_list[i].c_str(), 17);
    }

    p_onvif_device_list->devcie_list_lock = false;

    return 0;
}

ONVIFOPERATION_API int get_onvif_device_information(onvif_device_list* p_onvif_device_list, char* IP, size_t index)
{
    _tds__GetDeviceInformation          tds__GetDeviceInformation;
    _tds__GetDeviceInformationResponse  tds__GetDeviceInformationResponse;
    size_t i;

    while(p_onvif_device_list->devcie_list_lock)
    {
        Sleep(10);
    }
    p_onvif_device_list->devcie_list_lock = true;

    if(NULL != IP)
    {
        if(17 < strnlen(IP, 17))
        {
            p_onvif_device_list->devcie_list_lock = false;
            return -1;
        }
        for(i = 0; i < p_onvif_device_list->number_of_onvif_device; i++)
        {
            if(0 == strncmp(IP, p_onvif_device_list->p_onvif_device[i].IPv4, 17))
            {
                index = i;
            }
        }
    }

    if(p_onvif_device_list->number_of_onvif_device <= index)
    {
        p_onvif_device_list->devcie_list_lock = false;
        return -1;
    }


    soap_set_namespaces(pSoap, device_namespace);

    soap_wsse_add_UsernameTokenDigest(pSoap, "user", p_onvif_device_list->p_onvif_device[index].username, p_onvif_device_list->p_onvif_device[index].password);

    soap_call___tds__GetDeviceInformation(pSoap, p_onvif_device_list->p_onvif_device[index].device_service_address, NULL, &tds__GetDeviceInformation, tds__GetDeviceInformationResponse);

    strncpy(
        p_onvif_device_list->p_onvif_device[index].device_information.firmware_version,
        tds__GetDeviceInformationResponse.FirmwareVersion.c_str(),
        50);
    strncpy(
        p_onvif_device_list->p_onvif_device[index].device_information.hardware_Id,
        tds__GetDeviceInformationResponse.HardwareId.c_str(),
        10);
    strncpy(
        p_onvif_device_list->p_onvif_device[index].device_information.manufacturer,
        tds__GetDeviceInformationResponse.Manufacturer.c_str(),
        50);
    strncpy(
        p_onvif_device_list->p_onvif_device[index].device_information.model,
        tds__GetDeviceInformationResponse.Model.c_str(),
        50);
    strncpy(
        p_onvif_device_list->p_onvif_device[index].device_information.serial_number,
        tds__GetDeviceInformationResponse.SerialNumber.c_str(),
        50);

    p_onvif_device_list->devcie_list_lock = false;

    return 0;
}

ONVIFOPERATION_API int get_number_of_IPCs(void)
{
    if(!initialsuccess)
    {
        return -1;
    }

    while(device_list_locked)
    {
        Sleep(200);
    }
    device_list_locked = true;

    int number_of_IPCs = 0;

    //for(deviceInfoListIterator = deviceInfoList.begin(); deviceInfoListIterator != deviceInfoList.end(); ++deviceInfoListIterator)
    //{
    //    printf("%s\n", (*deviceInfoListIterator)->probeMatches.wsdd__ProbeMatches->ProbeMatch->Types);
    //    if(NULL == strncmp((*deviceInfoListIterator)->probeMatches.wsdd__ProbeMatches->ProbeMatch->Types, "dn:NetworkVideoTransmitter", strlen("dn:NetworkVideoTransmitter")))
    //    {
    //        ++number_of_IPCs;
    //    }
    //}

    number_of_IPCs = deviceInfoList.size();

    device_list_locked = false;
    return number_of_IPCs;
}

ONVIFOPERATION_API int get_number_of_NVRs(void)
{
    /*
    if(!initialsuccess)
    {
    return -1;
    }

    while(device_list_locked)
    {
    Sleep(200);
    }
    device_list_locked = true;

    int number_of_NVRs = 0;

    for(deviceInfoListIterator = deviceInfoList.begin(); deviceInfoListIterator != deviceInfoList.end(); ++deviceInfoListIterator)
    {
    if(0 == strncmp((*deviceInfoListIterator)->probeMatches.wsdd__ProbeMatches->ProbeMatch->Types, "dn:NetworkVideoTransmitter tds:Device", strlen("dn:NetworkVideoTransmitter tds:Device")))
    {
    ++number_of_NVRs;
    }
    }

    device_list_locked = false;
    return number_of_NVRs;
    */
    return -1;
}

ONVIFOPERATION_API int get_all_IPC_URIs(IPC_URI* IPC_URI_array, size_t num)
{
    while(device_list_locked)
    {
        Sleep(200);
    }
    device_list_locked = true;

    if(NULL == IPC_URI_array || !initialsuccess)
    {
        device_list_locked = false;
        return -1;
    }

    size_t i;
    regex expression("\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}");
    string strTemp;
    smatch match;

    for(deviceInfoListIterator = deviceInfoList.begin(), i = 0; deviceInfoListIterator != deviceInfoList.end(); ++deviceInfoListIterator, ++i)
    {
        memset(&IPC_URI_array[i], 0x0, sizeof(IPC_URI));
        if(NULL == (*deviceInfoListIterator)->probeMatches.wsdd__ProbeMatches)
        {
            continue;
        }
        if(NULL == (*deviceInfoListIterator)->probeMatches.wsdd__ProbeMatches->ProbeMatch)
        {
            continue;
        }
        //if(0 != strncmp((*deviceInfoListIterator)->probeMatches.wsdd__ProbeMatches->ProbeMatch->Types, "dn:NetworkVideoTransmitter", strlen("dn:NetworkVideoTransmitter")))
        //{
        //    continue;
        //}
        if(NULL == (*deviceInfoListIterator)->probeMatches.wsdd__ProbeMatches->ProbeMatch->XAddrs)
        {
            continue;
        }

        strTemp = (*deviceInfoListIterator)->probeMatches.wsdd__ProbeMatches->ProbeMatch->XAddrs;
        auto words_begin = sregex_iterator(strTemp.begin(), strTemp.end(), expression);
        auto words_end = sregex_iterator();

        for(sregex_iterator iterator = words_begin; iterator != words_end; ++iterator)
        {
            match = *iterator;
        }

        if(i >= num)
        {
            break;
        }

        strcpy(IPC_URI_array[i].ip, match.str().c_str());

        if(NULL == (*deviceInfoListIterator)->getStreamUriResponse.MediaUri)
        {
            continue;
        }

        strcpy(IPC_URI_array[i].URI, (*deviceInfoListIterator)->getStreamUriResponse.MediaUri->Uri.c_str());
    }

    device_list_locked = false;

    if(i > num)
    {
        return -1;
    }
    else
    {
        return num;
    }
}

ONVIFOPERATION_API int get_IPC_URI_according_to_IP(char* IP, size_t IPBufferLen, char* URI, size_t URLBufferLen, char* username, char* password)
{
    while(device_list_locked)
    {
        Sleep(200);
    }
    device_list_locked = true;

    if(NULL == IP || NULL == URI || NULL == username || NULL == password || strlen(IP) + 1 > IPBufferLen || !initialsuccess)
    {
        device_list_locked = false;
        return -1;
    }

    for(deviceInfoListIterator = deviceInfoList.begin(); deviceInfoListIterator != deviceInfoList.end(); ++deviceInfoListIterator)
    {
        if(NULL == (*deviceInfoListIterator)->probeMatches.wsdd__ProbeMatches)
        {
            continue;
        }
        if(NULL == (*deviceInfoListIterator)->probeMatches.wsdd__ProbeMatches->ProbeMatch)
        {
            continue;
        }
        //if(0 != strncmp((*deviceInfoListIterator)->probeMatches.wsdd__ProbeMatches->ProbeMatch->Types, "dn:NetworkVideoTransmitter", strlen("dn:NetworkVideoTransmitter")))
        //{
        //    continue;
        //}
        if(NULL == (*deviceInfoListIterator)->probeMatches.wsdd__ProbeMatches->ProbeMatch->XAddrs)
        {
            continue;
        }
        if(NULL != strstr((*deviceInfoListIterator)->probeMatches.wsdd__ProbeMatches->ProbeMatch->XAddrs, IP))
        {
            getRTSP(deviceInfoListIterator, username, password);
            break;
        }
    }

    if(deviceInfoListIterator == deviceInfoList.end())
    {
        device_list_locked = false;
        return -1;
    }

    if(NULL == (*deviceInfoListIterator)->getStreamUriResponse.MediaUri)
    {
        device_list_locked = false;
        return -1;
    }

    if(!strncpy(URI, (*deviceInfoListIterator)->getStreamUriResponse.MediaUri->Uri.c_str(), URLBufferLen))
    {
        device_list_locked = false;
        return -1;
    }

    device_list_locked = false;
    return 0;
}


ONVIFOPERATION_API int get_number_of_IPC_profiles_according_to_IP(char* IP, size_t IPBufferLen, char* username, char* password)
{
    while(device_list_locked)
    {
        Sleep(200);
    }
    device_list_locked = true;

    if(NULL == IP || strlen(IP) + 1 > IPBufferLen || !initialsuccess)
    {
        device_list_locked = false;
        return -1;
    }

    for(deviceInfoListIterator = deviceInfoList.begin(); deviceInfoListIterator != deviceInfoList.end(); ++deviceInfoListIterator)
    {
        if(NULL == (*deviceInfoListIterator)->probeMatches.wsdd__ProbeMatches)
        {
            continue;
        }
        if(NULL == (*deviceInfoListIterator)->probeMatches.wsdd__ProbeMatches->ProbeMatch)
        {
            continue;
        }
        if(NULL == (*deviceInfoListIterator)->probeMatches.wsdd__ProbeMatches->ProbeMatch->XAddrs)
        {
            continue;
        }
        if(NULL != strstr((*deviceInfoListIterator)->probeMatches.wsdd__ProbeMatches->ProbeMatch->XAddrs, IP))
        {
            break;
        }
    }

    if(deviceInfoListIterator == deviceInfoList.end())
    {
        device_list_locked = false;
        return -1;
    }

    int profilesSize;

    profilesSize = (*deviceInfoListIterator)->getProfilesResponse.Profiles.size();

    if(0 < profilesSize)
    {
        device_list_locked = false;
        return profilesSize;
    }

    if(-1 == getRTSP(deviceInfoListIterator, username, password))
    {
        device_list_locked = false;
        return -1;
    }

    profilesSize = (*deviceInfoListIterator)->getProfilesResponse.Profiles.size();

    if(0 < profilesSize)
    {
        device_list_locked = false;
        return profilesSize;
    }

    device_list_locked = false;
    return -1;
}

ONVIFOPERATION_API int get_IPC_profiles_according_to_IP(char *IP, size_t IPBufferLen, IPC_profiles* IPC_profiles_array, char* username, char* password)
{
    while(device_list_locked)
    {
        Sleep(200);
    }
    device_list_locked = true;

    if(NULL == IP || NULL == IPC_profiles_array || strlen(IP) + 1 > IPBufferLen)
    {
        device_list_locked = false;
        return -1;
    }

    for(deviceInfoListIterator = deviceInfoList.begin(); deviceInfoListIterator != deviceInfoList.end(); ++deviceInfoListIterator)
    {
        if(NULL == (*deviceInfoListIterator)->probeMatches.wsdd__ProbeMatches)
        {
            continue;
        }
        if(NULL == (*deviceInfoListIterator)->probeMatches.wsdd__ProbeMatches->ProbeMatch)
        {
            continue;
        }
        if(NULL == (*deviceInfoListIterator)->probeMatches.wsdd__ProbeMatches->ProbeMatch->XAddrs)
        {
            continue;
        }
        if(NULL != strstr((*deviceInfoListIterator)->probeMatches.wsdd__ProbeMatches->ProbeMatch->XAddrs, IP))
        {
            break;
        }
    }

    if(deviceInfoListIterator == deviceInfoList.end())
    {
        device_list_locked = false;
        return -1;
    }

    size_t i;
    _trt__GetStreamUriResponse _getStreamUriResponse;

    soap_set_namespaces(pSoap, getStreamUriNamespace);

    for(i = 0; i < (*deviceInfoListIterator)->getProfilesResponse.Profiles.size(); ++i)
    {
        memset(&IPC_profiles_array[i], 0x0, sizeof(IPC_profiles));
        IPC_profiles_array[i].encoding = (enum video_encoding)(*deviceInfoListIterator)->getProfilesResponse.Profiles[i]->VideoEncoderConfiguration->Encoding;
        IPC_profiles_array[i].frame_rate_limit = (*deviceInfoListIterator)->getProfilesResponse.Profiles[i]->VideoEncoderConfiguration->RateControl->FrameRateLimit;
        IPC_profiles_array[i].width = (*deviceInfoListIterator)->getProfilesResponse.Profiles[i]->VideoEncoderConfiguration->Resolution->Width;
        IPC_profiles_array[i].height = (*deviceInfoListIterator)->getProfilesResponse.Profiles[i]->VideoEncoderConfiguration->Resolution->Height;

        getStreamUri.ProfileToken = (*deviceInfoListIterator)->getProfilesResponse.Profiles[i]->token;
        soap_wsse_add_UsernameTokenDigest(pSoap, "user", username, password);
        soap_call___trt__GetStreamUri(pSoap, (*deviceInfoListIterator)->getCapabilitiesResponse.Capabilities->Media->XAddr.c_str(), NULL, &getStreamUri, _getStreamUriResponse);

        if(NULL == _getStreamUriResponse.MediaUri)
        {
            break;
        }
        strncpy(IPC_profiles_array[i].URI, _getStreamUriResponse.MediaUri->Uri.c_str(), sizeof(IPC_profiles_array[i].URI));
    }

    if((*deviceInfoListIterator)->getProfilesResponse.Profiles.size() != i)
    {
        device_list_locked = false;
        return -1;
    }
    else
    {
        device_list_locked = false;
        return i;
    }
}


ONVIFOPERATION_API void test(void)
{
    vector<tt__Profile*>::iterator it;
    for(deviceInfoListIterator = deviceInfoList.begin(); deviceInfoListIterator != deviceInfoList.end(); ++deviceInfoListIterator)
    {
        if(NULL == (*deviceInfoListIterator)->probeMatches.wsdd__ProbeMatches)
        {
            continue;
        }
        if(NULL == (*deviceInfoListIterator)->probeMatches.wsdd__ProbeMatches->ProbeMatch)
        {
            continue;
        }
        if(NULL == (*deviceInfoListIterator)->probeMatches.wsdd__ProbeMatches->ProbeMatch->XAddrs)
        {
            continue;
        }
        cout << "++++++++++++++++++++++++++++\n";
        printf("%s:::", (*deviceInfoListIterator)->probeMatches.wsdd__ProbeMatches->ProbeMatch->XAddrs);
        cout << (*deviceInfoListIterator)->getProfilesResponse.Profiles.size() << endl;
        for(it = (*deviceInfoListIterator)->getProfilesResponse.Profiles.begin(); it < (*deviceInfoListIterator)->getProfilesResponse.Profiles.end(); ++it)
        {
            switch((*it)->VideoEncoderConfiguration->Encoding)
            {
                case tt__VideoEncoding__JPEG:
                    cout << "Encoding: JPEG\n";
                    break;
                case tt__VideoEncoding__MPEG4:
                    cout << "Encoding: MPEG4\n";
                    break;
                case tt__VideoEncoding__H264:
                    cout << "Encoding: H264\n";
                    break;
                default:
                    cout << "Encoding: Unknow\n";
                    break;
            }
            cout << "Width: " << (*it)->VideoEncoderConfiguration->Resolution->Width << ' ' << "Height: " << (*it)->VideoEncoderConfiguration->Resolution->Height << endl;
            cout << "FrameRateLimit: " << (*it)->VideoEncoderConfiguration->RateControl->FrameRateLimit << ' ' << "BitrateLimit: " << (*it)->VideoEncoderConfiguration->RateControl->BitrateLimit << endl;
            cout << "****************\n";
        }
    }
}