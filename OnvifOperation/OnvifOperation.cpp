// OnvifOperation.cpp : Defines the exported functions for the DLL application.
//
#define _CRT_SECURE_NO_WARNINGS
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
}deviceInfoContainer;

static soap*                                  pSoap;
static wsdd__ScopesType                       scopes;
static SOAP_ENV__Header                       header;

static vector<deviceInfoContainer*>           deviceInfoList;
static vector<deviceInfoContainer*>::iterator deviceInfoListIterator;

static wsdd__ProbeType                        probe;
static _tds__GetCapabilities                  getCapabilities;
static _trt__GetProfiles                      getProfiles;
static _trt__GetStreamUri                     getStreamUri;

static tt__StreamSetup*                       pTemp1;
static tt__Transport*                         pTemp2;
static tt__Transport*                         pTemp3;

static bool initialsuccess = false;
static bool searchsuccess = false;

int getRTSP(vector<deviceInfoContainer*>::iterator& Iterator, char* username, char* password)
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

ONVIFOPERATION_API int initDll(void)
{
    if(0 != deviceInfoList.size())
    {
        for(deviceInfoListIterator = deviceInfoList.begin(); deviceInfoListIterator != deviceInfoList.end(); ++deviceInfoListIterator)
        {
            delete (*deviceInfoListIterator);
        }
        deviceInfoList.clear();
    }

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

ONVIFOPERATION_API int uninitDll(void)
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

    if(0 != deviceInfoList.size())
    {
        for(deviceInfoListIterator = deviceInfoList.begin(); deviceInfoListIterator != deviceInfoList.end(); ++deviceInfoListIterator)
        {
            delete (*deviceInfoListIterator);
        }
        deviceInfoList.clear();
    }

    soap_destroy(pSoap);
    soap_end(pSoap);
    soap_done(pSoap);

    pSoap = NULL;

    return 0;
}

ONVIFOPERATION_API int clearDeviceList(void)
{
    if(0 != deviceInfoList.size())
    {
        for(deviceInfoListIterator = deviceInfoList.begin(); deviceInfoListIterator != deviceInfoList.end(); ++deviceInfoListIterator)
        {
            delete (*deviceInfoListIterator);
        }
        deviceInfoList.clear();
    }
    return 0;
}

ONVIFOPERATION_API int resetDll(void)
{
    if(-1 == uninitDll())
    {
        return -1;
    }
    if(-1 == initDll())
    {
        return -1;
    }
    return 0;
}

ONVIFOPERATION_API int searchDev(void)
{
    if(!initialsuccess)
    {
        searchsuccess = false;
        return -1;
    }

    soap_default_SOAP_ENV__Header(pSoap, &header);
    soap_set_namespaces(pSoap, probeNamespace);
    pSoap->recv_timeout = 5;

    header.wsa__MessageID = (char*)soap_wsa_rand_uuid(pSoap);
    if(NULL == header.wsa__MessageID)
    {
        searchsuccess = false;
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

    if(SOAP_OK != soap_send___wsdd__Probe(pSoap, "soap.udp://239.255.255.250:3702", NULL, &probe))
    {
        searchsuccess = false;
        return -1;
    }

    // set not duplicated
    for(deviceInfoListIterator = deviceInfoList.begin(); deviceInfoListIterator != deviceInfoList.end(); ++deviceInfoListIterator)
    {
        (*deviceInfoListIterator)->duplicated = FALSE;
    }

    // get match result and put into vector
    while(TRUE)
    {
        deviceInfoContainer* pTempDeviceInfoContainer = new deviceInfoContainer;
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

    searchsuccess = true;

    return 0;
}

ONVIFOPERATION_API int getNumOfOnvifDev(void)
{
    if(!initialsuccess)
    {
        return -1;
    }

    if(!searchsuccess)
    {
        return -1;
    }

    return deviceInfoList.size();
}

ONVIFOPERATION_API int getURIFromIP(char* IP, size_t IPBufferLen, char* URI, size_t URIBufferLen, char* username, char* password)
{
    if(NULL == IP || NULL == URI || NULL == username || NULL == password)
    {
        return -1;
    }

    if(strlen(IP) + 1 > IPBufferLen)
    {
        return -1;
    }


    if(!initialsuccess)
    {
        return -1;
    }

    if(!searchsuccess)
    {
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
            getRTSP(deviceInfoListIterator, username, password);
            break;
        }
    }

    if(deviceInfoListIterator == deviceInfoList.end())
    {
        return -1;
    }

    if(NULL == (*deviceInfoListIterator)->getStreamUriResponse.MediaUri)
    {
        return -1;
    }

    if(!strncpy(URI, (*deviceInfoListIterator)->getStreamUriResponse.MediaUri->Uri.c_str(), URIBufferLen))
    {
        return -1;
    }

    return 0;
}

ONVIFOPERATION_API int getAllDevURI(deviceInfoArray* infoArray, size_t Num)
{
    if(NULL == infoArray)
    {
        return -1;
    }

    if(!initialsuccess)
    {
        return -1;
    }

    if(!searchsuccess)
    {
        return -1;
    }

    if(Num != deviceInfoList.size())
    {
        return -1;
    }

    size_t i;
    regex expression("\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}");
    string strTemp;
    smatch match;

    for(deviceInfoListIterator = deviceInfoList.begin(), i = 0; deviceInfoListIterator != deviceInfoList.end(); ++deviceInfoListIterator, ++i)
    {
        memset(&infoArray[i], 0x0, sizeof(deviceInfoArray));
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

        strTemp = (*deviceInfoListIterator)->probeMatches.wsdd__ProbeMatches->ProbeMatch->XAddrs;
        auto words_begin = sregex_iterator(strTemp.begin(), strTemp.end(), expression);
        auto words_end = sregex_iterator();

        for(sregex_iterator iterator = words_begin; iterator != words_end; ++iterator)
        {
            match = *iterator;
        }

        strcpy(infoArray[i].ip, match.str().c_str());

        if(NULL == (*deviceInfoListIterator)->getStreamUriResponse.MediaUri)
        {
            continue;
        }

        strcpy(infoArray[i].URI, (*deviceInfoListIterator)->getStreamUriResponse.MediaUri->Uri.c_str());
    }
    return Num;
}

ONVIFOPERATION_API int getNumOfProfilesFromIP(char* IP, size_t IPBufferLen, char* username, char* password)
{
    if(NULL == IP)
    {
        return -1;
    }

    if(strlen(IP) + 1 > IPBufferLen)
    {
        return -1;
    }

    if(!initialsuccess)
    {
        return -1;
    }

    if(!searchsuccess)
    {
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
        return -1;
    }

    if(0 < (*deviceInfoListIterator)->getProfilesResponse.Profiles.size())
    {
        return (*deviceInfoListIterator)->getProfilesResponse.Profiles.size();
    }

    if(-1 == getRTSP(deviceInfoListIterator, username, password))
    {
        return -1;
    }

    if(0 < (*deviceInfoListIterator)->getProfilesResponse.Profiles.size())
    {
        return (*deviceInfoListIterator)->getProfilesResponse.Profiles.size();
    }

    return -1;
}

ONVIFOPERATION_API int getVideoInfoFromIP(char *IP, size_t IPBufferLen, videoNode *headVideo, char* username, char* password)
{
    if(NULL == IP || NULL == headVideo)
    {
        return -1;
    }

    if(strlen(IP) + 1 > IPBufferLen)
    {
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
        return -1;
    }

    size_t i;
    _trt__GetStreamUriResponse _getStreamUriResponse;

    soap_set_namespaces(pSoap, getStreamUriNamespace);

    for(i = 0; i < (*deviceInfoListIterator)->getProfilesResponse.Profiles.size(); ++i)
    {
        memset(&headVideo[i], 0x0, sizeof(videoNode));
        headVideo[i].encoding = (enum videoEncoding)(*deviceInfoListIterator)->getProfilesResponse.Profiles[i]->VideoEncoderConfiguration->Encoding;
        headVideo[i].frame = (*deviceInfoListIterator)->getProfilesResponse.Profiles[i]->VideoEncoderConfiguration->RateControl->FrameRateLimit;
        headVideo[i].width = (*deviceInfoListIterator)->getProfilesResponse.Profiles[i]->VideoEncoderConfiguration->Resolution->Width;
        headVideo[i].height = (*deviceInfoListIterator)->getProfilesResponse.Profiles[i]->VideoEncoderConfiguration->Resolution->Height;

        getStreamUri.ProfileToken = (*deviceInfoListIterator)->getProfilesResponse.Profiles[i]->token;
        soap_wsse_add_UsernameTokenDigest(pSoap, "user", username, password);
        soap_call___trt__GetStreamUri(pSoap, (*deviceInfoListIterator)->getCapabilitiesResponse.Capabilities->Media->XAddr.c_str(), NULL, &getStreamUri, _getStreamUriResponse);

        strncpy(headVideo[i].URI, _getStreamUriResponse.MediaUri->Uri.c_str(), sizeof(headVideo[i].URI));
    }

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