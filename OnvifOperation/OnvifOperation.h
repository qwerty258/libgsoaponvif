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

    // device info struct
    typedef struct
    {
        char ip[17];
        char URI[256];
    }IPC_URI;

    // VideoEncoding
    enum video_encoding
    {
        videoEncoding__JPEG = 0,
        videoEncoding__MPEG4 = 1,
        videoEncoding__H264 = 2
    };

    // video info struct
    typedef struct
    {
        char URI[256];
        int frame;
        int width;
        int height;
        enum video_encoding encoding;
    }IPC_profiles;

    //function : initial dll, locate some memory
    //on return: 0 success, -1 failure
    //thread unsafe
    ONVIFOPERATION_API int init_DLL(void);

    //function : uninitial dll
    //on return: 0 success, -1 failure
    //thread unsafe
    ONVIFOPERATION_API int uninit_DLL(void);

    //function : reset everything in dll, think before use this function!
    //on return: 0 success, -1 failure. if failed, reload the DLL.
    //thread unsafe
    ONVIFOPERATION_API int reset_DLL(void);

    //function : search onvif device
    //on return: 0 success, -1 failure
    //thread safe
    //waitTime : interval for cameras to response, in seconds
    ONVIFOPERATION_API int search_ONVIF_IPC(size_t waitTime);

    //function : clear the onvif device list maintained by this DLL
    //on return: 0 success, -1 failure.
    //thread safe
    ONVIFOPERATION_API int clear_device_list(void);

    //function : get number of onvif IPC
    //on return: number of onvif IPC, -1 something went wrong
    //thread safe
    ONVIFOPERATION_API int get_number_of_IPCs(void);

    //function : get number of onvif NVR
    //on return: number of onvif NVR, -1 something went wrong
    //thread safe
    ONVIFOPERATION_API int get_number_of_NVRs(void);

    //function : get all onvif device URI
    //on return: the number of onvif device info put into deviceInfoArray, -1 failure
    //thread safe
    /* !!!! YOU SHOULD ALLOCATE THE IPC_URI_array BUFFER !!!! */
    //Num          : number of all onvif IPC, returned by get_number_of_IPCs
    //IPC_URI_array: pointer to the head of IPC_URI_array buffer you allocated
    ONVIFOPERATION_API int get_all_IPC_URIs(IPC_URI* IPC_URI_array, size_t num);

    //function : get the main URI according to IP
    //on return: 0 success, -1 failure
    //thread safe
    /* !!!! YOU SHOULD ALLOCATE THE URI BUFFER !!!! */
    //IP          : pointer to the head of C-style IP string buffer
    //IPBufferLen : IP string buffer size, in bytes
    //username    : C-style string of user name
    //password    : C-sytle string of pass word
    //URI         : pointer to the head of C-style URI string buffer you allocated
    //URIBufferLen: URI string buffer size, in bytes
    ONVIFOPERATION_API int get_IPC_URI_according_to_IP(char* IP, size_t IPBufferLen, char* URI, size_t URLBufferLen, char* username = "", char* password = "");

    //function : get the number of profiles according to IP
    //on return: the number of profiles, -1 failure
    //thread safe
    //IP          : pointer to the head of C-style IP string
    //IPBufferLen : IP string buffer size, in bytes
    //username    : C-style string of user name
    //password    : C-sytle string of pass word
    ONVIFOPERATION_API int get_number_of_IPC_profiles_according_to_IP(char* IP, size_t IPBufferLen, char* username = "", char* password = "");

    //function : get the array of profiles according to IP
    //on return: the number of profiles, -1 failure
    //thread safe
    /* !!!! YOU SHOULD ALLOCATE THE IPC_profiles_array BUFFER !!!! */
    //IP          : pointer to the head of C-style IP string
    //IPBufferLen : IP string buffer size, in bytes
    //username    : C-style string of user name
    //password    : C-sytle string of pass word
    //IPC_profiles_array: pointer to the head of IPC_profiles_array buffer you allocated
    ONVIFOPERATION_API int get_IPC_profiles_according_to_IP(char* IP, size_t IPBufferLen, IPC_profiles* IPC_profiles_array, char* username = "", char* password = "");

    ONVIFOPERATION_API void test(void);

#ifdef __cplusplus
}
#endif // __cplusplus
