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

typedef struct tag_onvif_device_service_address
{
    char namesapce[256];
    char xaddr[256];
    int major_version;
    int minor_version;
}onvif_device_service_address;

typedef struct tag_onvif_device_information
{
    char manufacturer[50];
    char model[50];
    char firmware_version[50];
    char serial_number[50];
    char hardware_Id[10];
}onvif_device_information;

typedef struct tag_onvif_device
{
    //************************************
    // For informaiton preserve purpose.
    // User MUST NOT set this content. Read only.
    //************************************
    bool duplicated;

    //************************************
    // IPv4 of the onvif device, set by search_ONVIF_device().
    // User MUST NOT set this content. Read only.
    //************************************
    char IPv4[17];

    //************************************
    // onvif device service addresses, set by search_ONVIF_device() and other API.
    // User MUST NOT set this content. Read only.
    //************************************
    onvif_device_service_address service_address_device_service;
    onvif_device_service_address service_address_media;
    onvif_device_service_address service_address_events;
    onvif_device_service_address service_address_PTZ;
    onvif_device_service_address service_address_imaging;
    onvif_device_service_address service_address_deviceIO;
    onvif_device_service_address service_address_analytics;
    onvif_device_service_address service_address_recording;
    onvif_device_service_address service_address_search_recording;
    onvif_device_service_address service_address_replay;

    //************************************
    // device authorization information.
    // User MUST set this content with C-style string after search_ONVIF_device().
    // If there is no username and password, copy empty C-style string to it.
    //************************************
    char username[50];
    char password[50];

    //************************************
    // device basic information.
    // User MUST NOT set this content. Read only.
    //************************************
    onvif_device_information device_information;
}onvif_device;

typedef struct tag_onvif_device_list
{
    onvif_device* p_onvif_device;
    size_t number_of_onvif_device;
    bool devcie_list_lock;
}onvif_device_list;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


    //************************************
    // function:  initial DLL, locate some memory; call once per process.
    // Returns:   int: 0 success, -1 failure.
    // Parameter: void.
    //************************************
    ONVIFOPERATION_API int init_DLL(void);


    //************************************
    // function:  uninitial DLL; call once per process.
    // Returns:   int: 0 success, -1 failure.
    // Parameter: void.
    //************************************
    ONVIFOPERATION_API int uninit_DLL(void);


    //************************************
    // function:  get a pointer to the device list head.
    // Returns:   a pointer to the device list head, NULL if failed.
    // Parameter: void.
    //************************************
    ONVIFOPERATION_API onvif_device_list* malloc_device_list(void);


    //************************************
    // function:  free the device list head.
    // Returns:   void.
    // Parameter: pointer to the pointer which points to the device list.
    //************************************
    ONVIFOPERATION_API void free_device_list(onvif_device_list** pp_onvif_device_list);


    //************************************
    // function:  search onvif device. if you search again, the information in onvif_device_list will be lost; you need get it again.
    // Returns:   int: 0 success, -1 failure
    // Parameter: onvif_device_list* p_onvif_device_list: pointer get from malloc_device_list(void).
    // Parameter: int waitTime: interval for cameras to response, when > 0, gives socket recv timeout in seconds, < 0 in usec.
    //************************************
    ONVIFOPERATION_API int search_ONVIF_device(onvif_device_list* p_onvif_device_list, int wait_time);

    //************************************
    // function:  get onvif device information.
    // Returns:   int: 0 success, -1 failure.
    // Parameter: onvif_device_list* p_onvif_device_list: pointer get from malloc_device_list(void).
    // Parameter: char* IP: the IPC's IP you want to operate, or you can use index get from onvif_device_list.
    // Parameter: size_t index: index of onvif_device array, if char* IP is not NULL, this parameter will be ignored.
    //************************************
    ONVIFOPERATION_API int get_onvif_device_information(onvif_device_list* p_onvif_device_list, char* IP, size_t index);

    //************************************
    // function:  get onvif device service addresses.
    // Returns:   int: 0 success, -1 failure.
    // Parameter: onvif_device_list* p_onvif_device_list: pointer get from malloc_device_list(void).
    // Parameter: char* IP: the IPC's IP you want to operate, or you can use index get from onvif_device_list.
    // Parameter: size_t index: index of onvif_device array, if char* IP is not NULL, this parameter will be ignored.
    //************************************
    ONVIFOPERATION_API int get_onvif_device_service_address(onvif_device_list* p_onvif_device_list, char* IP, size_t index);

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
