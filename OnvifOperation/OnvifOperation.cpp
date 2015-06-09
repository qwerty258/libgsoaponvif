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
static bool searchsuccess = false;
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

    soap_set_namespaces(pSoap, getCapabilitiesNamespace);

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
    clear_device_list();

    pSoap = soap_new1(SOAP_IO_DEFAULT | SOAP_XML_IGNORENS); // ignore namespace, avoid namespace mismatch
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

    clear_device_list();

    delete pTemp3;
    delete pTemp2;
    delete pTemp1;

    pTemp3 = NULL;
    pTemp2 = NULL;
    pTemp1 = NULL;

    soap_destroy(pSoap);
    soap_end(pSoap);
    soap_done(pSoap);

    pSoap = NULL;

    initialsuccess = false;
    searchsuccess = false;

    return 0;
}

ONVIFOPERATION_API int reset_DLL(void)
{
    if(-1 == uninit_DLL())
    {
        return -1;
    }
    if(-1 == init_DLL())
    {
        return -1;
    }

    return 0;
}

ONVIFOPERATION_API int search_ONVIF_IPC(size_t waitTime)
{
    while(device_list_locked)
    {
        Sleep(200);
    }
    device_list_locked = true;


    if(!initialsuccess)
    {
        searchsuccess = false;
        device_list_locked = false;
        return -1;
    }

    soap_default_SOAP_ENV__Header(pSoap, &header);
    soap_set_namespaces(pSoap, probeNamespace);
    pSoap->recv_timeout = waitTime;

    header.wsa__MessageID = (char*)soap_wsa_rand_uuid(pSoap);
    if(NULL == header.wsa__MessageID)
    {
        searchsuccess = false;
        device_list_locked = false;
        return -1;
    }

    header.wsa__To = "urn:schemas-xmlsoap-org:ws:2005:04:discovery";
    header.wsa__Action = "http://schemas.xmlsoap.org/ws/2005/04/discovery/Probe";
    pSoap->header = &header;

    soap_default_wsdd__ScopesType(pSoap, &scopes);
    scopes.__item = "onvif://www.onvif.org";
    soap_default_wsdd__ProbeType(pSoap, &probe);
    probe.Scopes = &scopes;
    probe.Types = "ns1:NetworkVideoTransmitter";
    //probe.Types = "NetworkVideoTransmitter";

    // set not duplicated
    for(deviceInfoListIterator = deviceInfoList.begin(); deviceInfoListIterator != deviceInfoList.end(); ++deviceInfoListIterator)
    {
        (*deviceInfoListIterator)->duplicated = FALSE;
    }

    if(SOAP_OK != soap_send___wsdd__Probe(pSoap, "soap.udp://239.255.255.250:3702", NULL, &probe))
    {
        searchsuccess = false;
        device_list_locked = false;
        return -1;
    }

    // get match result and put into vector
    while(TRUE)
    {
        device_info_container* pTempDeviceInfoContainer = new device_info_container;
        if(SOAP_OK != soap_recv___wsdd__ProbeMatches(pSoap, &pTempDeviceInfoContainer->probeMatches))
        {
            delete pTempDeviceInfoContainer;
            break;
        }
        else
        {
            if(NULL == pTempDeviceInfoContainer->probeMatches.wsdd__ProbeMatches)
            {
                continue;
            }
            if(NULL == pTempDeviceInfoContainer->probeMatches.wsdd__ProbeMatches->ProbeMatch)
            {
                continue;
            }
            if(NULL == pTempDeviceInfoContainer->probeMatches.wsdd__ProbeMatches->ProbeMatch->XAddrs)
            {
                continue;
            }

            // check result
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

                if(0 == strncmp((*deviceInfoListIterator)->probeMatches.wsdd__ProbeMatches->ProbeMatch->XAddrs, pTempDeviceInfoContainer->probeMatches.wsdd__ProbeMatches->ProbeMatch->XAddrs, 22))
                {
                    (*deviceInfoListIterator)->duplicated = TRUE;
                    break;
                }
            }

            // if not in the previous result, push back
            if(deviceInfoListIterator == deviceInfoList.end())
            {
                pTempDeviceInfoContainer->duplicated = TRUE;
                deviceInfoList.push_back(pTempDeviceInfoContainer);
            }
        }
    }

    // remove removed device info
    deviceInfoListIterator = deviceInfoList.begin();
    while(deviceInfoListIterator != deviceInfoList.end())
    {
        for(deviceInfoListIterator = deviceInfoList.begin(); deviceInfoListIterator != deviceInfoList.end(); ++deviceInfoListIterator)
        {
            if(!(*deviceInfoListIterator)->duplicated)
            {
                delete (*deviceInfoListIterator);
                deviceInfoList.erase(deviceInfoListIterator);
                break;
            }
        }
    }

    // try get rtsp
    for(deviceInfoListIterator = deviceInfoList.begin(); deviceInfoListIterator != deviceInfoList.end(); ++deviceInfoListIterator)
    {
        if(NULL != (*deviceInfoListIterator)->getStreamUriResponse.MediaUri)
        {
            if(1 < (*deviceInfoListIterator)->getStreamUriResponse.MediaUri->Uri.size())
            {
                continue;
            }
        }
        getRTSP(deviceInfoListIterator, "", "");
    }

    device_list_locked = false;
    searchsuccess = true;

    return 0;
}

ONVIFOPERATION_API int clear_device_list(void)
{
    while(device_list_locked)
    {
        Sleep(200);
    }
    device_list_locked = true;

    if(0 != deviceInfoList.size())
    {
        for(deviceInfoListIterator = deviceInfoList.begin(); deviceInfoListIterator != deviceInfoList.end(); ++deviceInfoListIterator)
        {
            delete (*deviceInfoListIterator);
        }
        deviceInfoList.clear();
    }

    device_list_locked = false;
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

    if(NULL == IPC_URI_array || !initialsuccess || !searchsuccess)
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

    if(NULL == IP || NULL == URI || NULL == username || NULL == password || strlen(IP) + 1 > IPBufferLen || !initialsuccess || !searchsuccess)
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

    if(NULL == IP || strlen(IP) + 1 > IPBufferLen || !initialsuccess || !searchsuccess)
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
        IPC_profiles_array[i].frame = (*deviceInfoListIterator)->getProfilesResponse.Profiles[i]->VideoEncoderConfiguration->RateControl->FrameRateLimit;
        IPC_profiles_array[i].width = (*deviceInfoListIterator)->getProfilesResponse.Profiles[i]->VideoEncoderConfiguration->Resolution->Width;
        IPC_profiles_array[i].height = (*deviceInfoListIterator)->getProfilesResponse.Profiles[i]->VideoEncoderConfiguration->Resolution->Height;

        getStreamUri.ProfileToken = (*deviceInfoListIterator)->getProfilesResponse.Profiles[i]->token;
        soap_wsse_add_UsernameTokenDigest(pSoap, "user", username, password);
        soap_call___trt__GetStreamUri(pSoap, (*deviceInfoListIterator)->getCapabilitiesResponse.Capabilities->Media->XAddr.c_str(), NULL, &getStreamUri, _getStreamUriResponse);

        strncpy(IPC_profiles_array[i].URI, _getStreamUriResponse.MediaUri->Uri.c_str(), sizeof(IPC_profiles_array[i].URI));
    }

    device_list_locked = false;
    return i;
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