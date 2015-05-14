/**************************************************************
 *dll usage:
 *initDll first(get some memory from system), then searchDev.
 *Now you can getNumOfOnvifDev, getURIFromIP, getAllDevURI.
 *If you added new onvif device, call searchDev again and you
 *sholud found new device using getURIFromIP and getAllDevURI.
 *when you done, please call uninitDll to release memory.
 **************************************************************/

/**************************************************************
 *If there is VMware adapter, please disable it; otherwise
 *multicast package will end up somewhere unkonw.
 *Allow the program through fire wall.
 **************************************************************/

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the ONVIFOPERATION_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// ONVIFOPERATION_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.

#pragma once

#ifdef ONVIFOPERATION_EXPORTS
#define ONVIFOPERATION_API __declspec(dllexport)
#else
#define ONVIFOPERATION_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

    //listOfURI
    typedef struct
    {
        char ip[17];
        char URI[256];
    }deviceInfoArray;

    //function : initial dll, locate some memory
    //on return: 0 success, -1 failure
    int ONVIFOPERATION_API initDll(void);

    //function : uninitial dll
    //on return: 0 success, -1 failure
    int ONVIFOPERATION_API uninitDll(void);

    //function : search onvif device
    //on return: 0 success, -1 failure
    int ONVIFOPERATION_API searchDev(void);

    //function : get number of onvif device
    //on return: number of onvif device, -1 something went wrong
    int ONVIFOPERATION_API getNumOfOnvifDev(void);

    //function : get the URI specified by caller according to IP
    /****************************************************
     you should locate all the memory this function needs
     ****************************************************/
    //input    :
    //////IP          : pointer to the head of C-style IP string
    //////IPBufferLen : bytes of the IP string
    //////username    : C-style string of user name
    //////password    : C-sytle string of pass word
    //output   :
    //////URI         : pointer to the head of C-style URI string
    //////URIBufferLen: bytes of the URI string
    //on return: 0 success, -1 failure
    int ONVIFOPERATION_API getURIFromIP(char* IP, size_t IPBufferLen, char* URI, size_t URIBufferLen, char* username, char* password);

    //function : get all onvif device URI
    /****************************************************
     you should locate all the memory this function needs
     ****************************************************/
    //input    : Num: number of all onvif device
    //output   : nodeList: pointer to the head of deviceInfoArray
    //on return: the number of onvif device info put into deviceInfoArray, -1 failure
    int ONVIFOPERATION_API getAllDevURI(deviceInfoArray* infoArray, size_t Num);

    //function : clear the onvif device list maintained by this DLL
    //on return: 0 success, -1 failure.
    int ONVIFOPERATION_API clearDeviceList(void);

    //function : reset everything in dll, think before use this function!
    //on return: 0 success, -1 failure. if failed, reload the DLL.
    int ONVIFOPERATION_API resetDll(void);

#ifdef __cplusplus
}
#endif // __cplusplus
