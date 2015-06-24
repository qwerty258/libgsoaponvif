// multiThreadTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <OnvifOperation.h>
#pragma comment(lib,"OnvifOperation.lib")

DWORD WINAPI getServiceAddresses(LPVOID lpParameter)
{
    std::cout << "Thread getServiceAddresses begin\n";

    for(size_t i = 0; i < static_cast<onvif_device_list*>(lpParameter)->number_of_onvif_device; i++)
    {
        get_onvif_device_service_address(static_cast<onvif_device_list*>(lpParameter), NULL, i);
    }

    std::cout << "Thread getServiceAddresses end\n";
    return 0;
}

DWORD WINAPI searchDevice(LPVOID lpParameter)
{
    std::cout << "Thread searchDevice begin\n";

    search_ONVIF_device(static_cast<onvif_device_list*>(lpParameter), 1);

    std::cout << "Thread searchDevice end\n";

    return 0;
}

DWORD WINAPI getDeviceInformation(LPVOID lpParameter)
{
    std::cout << "Thread getURIbyIP begin\n";

    for(size_t i = 0; i < static_cast<onvif_device_list*>(lpParameter)->number_of_onvif_device; i++)
    {
        get_onvif_device_information(static_cast<onvif_device_list*>(lpParameter), NULL, i);
    }

    std::cout << "Thread getURIbyIP end\n";
    return 0;
}

DWORD WINAPI getDeviceProfiles(LPVOID lpParameter)
{
    std::cout << "Thread getNumOfProfilesByIP begin\n";

    for(size_t i = 0; i < static_cast<onvif_device_list*>(lpParameter)->number_of_onvif_device; i++)
    {
        get_onvif_device_profiles(static_cast<onvif_device_list*>(lpParameter), NULL, i);
    }

    std::cout << "Thread getNumOfProfilesByIP end\n";
    return 0;
}

typedef struct
{
    char IP[17];
    char username[50];
    char password[50];
}struct_IP_username_password;

int _tmain(int argc, _TCHAR* argv[])
{
    HANDLE handleArray[5];
    onvif_device_list* p_onvif_device_list = malloc_device_list();
    struct_IP_username_password IP_username_password[8] =
    {
        "192.168.10.185", "admin", "12345",
        "192.168.10.141", "admin", "12345",
        "192.168.10.146", "admin", "Tolendata",
        "192.168.10.182", "admin", "Tolendata",
        "192.168.10.149", "admin", "Tolendata",
        "192.168.10.181", "admin", "Tolendata",
        "192.168.10.195", "admin", "12345",
        "192.168.10.142", "admin", "12345"
    };

    init_DLL();

    search_ONVIF_device(p_onvif_device_list, 1);

    for(size_t i = 0; i < p_onvif_device_list->number_of_onvif_device; ++i)
    {
        for(size_t j = 0; j < 8; j++)
        {
            if(0 == strncmp(p_onvif_device_list->p_onvif_device[i].IPv4, IP_username_password[j].IP, 17))
            {
                strncpy(p_onvif_device_list->p_onvif_device[i].username, IP_username_password[j].username, 50);
                strncpy(p_onvif_device_list->p_onvif_device[i].password, IP_username_password[j].password, 50);
            }
        }
    }

    handleArray[1] = CreateThread(NULL, 0, searchDevice, p_onvif_device_list, 0, NULL);
    handleArray[0] = CreateThread(NULL, 0, getServiceAddresses, p_onvif_device_list, 0, NULL);
    handleArray[2] = CreateThread(NULL, 0, getDeviceInformation, p_onvif_device_list, 0, NULL);
    handleArray[3] = CreateThread(NULL, 0, getDeviceProfiles, p_onvif_device_list, 0, NULL);
    handleArray[4] = CreateThread(NULL, 0, searchDevice, p_onvif_device_list, 0, NULL);

    WaitForMultipleObjects(5, handleArray, TRUE, INFINITE);

    uninit_DLL();

    free_device_list(&p_onvif_device_list);

    system("pause");

    return 0;
}

