//************************************
// DLL usage :
// init_DLL first(get some memory from system), then search_ONVIF_device.
// Now you can get_number_of_IPCs, get_IPC_URI_according_to_IP, get_all_IPC_URIs.
// If you added new onvif device, call search_ONVIF_device again and you
// should found new device using get_IPC_URI_according_to_IP and get_all_IPC_URIs.
// when you done, please call uninit_DLL to release memory.
//************************************

//************************************
// If there is VMware adapter, please disable it;
// otherwise multicast package will end up somewhere unknown.
// Allow the program through fire wall.
//************************************

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
        int frame_rate_limit;
        int width;
        int height;
        enum video_encoding encoding;
    }IPC_profiles;

    //************************************
    // function:  initial DLL, locate some memory. thread unsafe
    // Returns:   int: 0 success, -1 failure
    // Parameter: void
    //************************************
    ONVIFOPERATION_API int init_DLL(void);

    //************************************
    // function:  uninitial DLL. thread unsafe
    // Returns:   int: 0 success, -1 failure
    // Parameter: void
    //************************************
    ONVIFOPERATION_API int uninit_DLL(void);

    //************************************
    // function:  reset everything in DLL, think before use this function! thread unsafe
    // Returns:   int: 0 success, -1 failure. if failed, reload the DLL.
    // Parameter: void
    //************************************
    ONVIFOPERATION_API int reset_DLL(void);

    //************************************
    // function:  search onvif device. thread safe
    // Returns:   int: 0 success, -1 failure
    // Parameter: int waitTime: interval for cameras to response, when > 0, gives socket recv timeout in seconds, < 0 in usec
    //************************************
    ONVIFOPERATION_API int search_ONVIF_device(int waitTime);

    //************************************
    // function:  clear the onvif device list maintained by this DLL. thread safe
    // Returns:   int: 0 success, -1 failure.
    // Parameter: void
    //************************************
    ONVIFOPERATION_API int clear_device_list(void);

    //************************************
    // function:  get number of onvif IPC. thread safe
    // FullName:  get_number_of_IPCs
    // Returns:   int: number of onvif IPC, -1 something went wrong
    // Parameter: void
    //************************************
    ONVIFOPERATION_API int get_number_of_IPCs(void);

    //************************************
    // function:  get number of onvif NVR. thread safe
    // Returns:   int: number of onvif NVR, -1 something went wrong
    // Parameter: void
    //************************************
    ONVIFOPERATION_API int get_number_of_NVRs(void);

    //************************************
    // function:  get all onvif device URI. thread safe
    // Returns:   int: the number of onvif device info put into deviceInfoArray, -1 failure
    // Parameter: IPC_URI * IPC_URI_array: pointer to the head of IPC_URI_array buffer you allocated
    // Parameter: size_t num: number of all onvif IPC, returned by get_number_of_IPCs
    //************************************
    ONVIFOPERATION_API int get_all_IPC_URIs(IPC_URI* IPC_URI_array, size_t num);

    //************************************
    // function:  get the main URI according to IP. thread safe
    // Returns:   int: 0 success, -1 failure
    // Parameter: char * IP: pointer to the head of C-style IP string buffer
    // Parameter: size_t IPBufferLen: IP string buffer size, in bytes
    // Parameter: char * URI: pointer to the head of C-style URI string buffer you allocated
    // Parameter: size_t URLBufferLen: URI string buffer size, in bytes
    // Parameter: char * username: C-style string of username
    // Parameter: char * password: C-sytle string of password
    //************************************
    ONVIFOPERATION_API int get_IPC_URI_according_to_IP(char* IP, size_t IPBufferLen, char* URI, size_t URLBufferLen, char* username = "", char* password = "");

    //************************************
    // function:  get the number of profiles according to IP. thread safe
    // Returns:   int: the number of profiles, -1 failure
    // Parameter: char * IP: pointer to the head of C-style IP string
    // Parameter: size_t IPBufferLen: IP string buffer size, in bytes
    // Parameter: char * username: C-style string of username
    // Parameter: char * password: C-sytle string of password
    //************************************
    ONVIFOPERATION_API int get_number_of_IPC_profiles_according_to_IP(char* IP, size_t IPBufferLen, char* username = "", char* password = "");

    //************************************
    // function:  get the array of profiles according to IP. thread safe
    // Returns:   int: the number of profiles, -1 failure
    // Parameter: char * IP: pointer to the head of C-style IP string
    // Parameter: size_t IPBufferLen: IP string buffer size, in bytes
    // Parameter: IPC_profiles * IPC_profiles_array: pointer to the head of IPC_profiles_array buffer you allocated
    // Parameter: char * username: C-style string of username
    // Parameter: char * password: C-sytle string of password
    //************************************
    ONVIFOPERATION_API int get_IPC_profiles_according_to_IP(char* IP, size_t IPBufferLen, IPC_profiles* IPC_profiles_array, char* username = "", char* password = "");

    ONVIFOPERATION_API void test(void);

#ifdef __cplusplus
}
#endif // __cplusplus
