#include <Windows.h>
#include <OnvifOperation.h>
#include <iostream>
using namespace std;

DWORD WINAPI getServiceAddresses(LPVOID lpParameter)
{
    cout << "Thread getServiceAddresses begin\n";

    for(size_t i = 0; i < static_cast<onvif_device_list*>(lpParameter)->number_of_onvif_devices; i++)
    {
        get_onvif_device_service_addresses(static_cast<onvif_device_list*>(lpParameter), NULL, i);
    }

    cout << "Thread getServiceAddresses end\n";
    return 0;
}

DWORD WINAPI searchDevice(LPVOID lpParameter)
{
    cout << "Thread searchDevice begin\n";

    search_onvif_device(static_cast<onvif_device_list*>(lpParameter), 1);

    cout << "Thread searchDevice end\n";

    return 0;
}

DWORD WINAPI getDeviceInformation(LPVOID lpParameter)
{
    cout << "Thread getURIbyIP begin\n";

    for(size_t i = 0; i < static_cast<onvif_device_list*>(lpParameter)->number_of_onvif_devices; i++)
    {
        get_onvif_device_information(static_cast<onvif_device_list*>(lpParameter), NULL, i);
    }

    cout << "Thread getURIbyIP end\n";
    return 0;
}

DWORD WINAPI getDeviceProfiles(LPVOID lpParameter)
{
    cout << "Thread getNumOfProfilesByIP begin\n";

    for(size_t i = 0; i < static_cast<onvif_device_list*>(lpParameter)->number_of_onvif_devices; i++)
    {
        get_onvif_ipc_profiles(static_cast<onvif_device_list*>(lpParameter), NULL, i);
    }

    cout << "Thread getNumOfProfilesByIP end\n";
    return 0;
}

typedef struct
{
    char IP[17];
    char username[50];
    char password[50];
}struct_IP_username_password;

int main(int argc, char* argv[])
{
    HANDLE handleArray[5];
    onvif_device_list* p_onvif_device_list = NULL;
    struct_IP_username_password IP_username_password[9] =
    {
        "192.168.10.185", "admin", "12345",
        "192.168.10.141", "admin", "12345",
        "192.168.10.146", "admin", "Tolendata",
        "192.168.10.182", "admin", "Tolendata",
        "192.168.10.149", "admin", "Tolendata",
        "192.168.10.181", "admin", "Tolendata",
        "192.168.10.195", "admin", "12345",
        "192.168.10.142", "admin", "12345",
        "192.168.10.231", "admin", "Tolendata"
    };

    init_DLL();

    p_onvif_device_list = malloc_device_list();

    search_onvif_device(p_onvif_device_list, 1);

    for(size_t i = 0; i < p_onvif_device_list->number_of_onvif_devices; ++i)
    {
        for(size_t j = 0; j < 9; j++)
        {
            if(0 == strncmp(p_onvif_device_list->p_onvif_devices[i].IPv4, IP_username_password[j].IP, 17))
            {
                strncpy(p_onvif_device_list->p_onvif_devices[i].username, IP_username_password[j].username, 50);
                strncpy(p_onvif_device_list->p_onvif_devices[i].password, IP_username_password[j].password, 50);
            }
        }
        get_onvif_device_service_addresses(p_onvif_device_list, NULL, i);
    }


    handleArray[1] = CreateThread(NULL, 0, searchDevice, p_onvif_device_list, 0, NULL);
    handleArray[0] = CreateThread(NULL, 0, getServiceAddresses, p_onvif_device_list, 0, NULL);
    handleArray[2] = CreateThread(NULL, 0, getDeviceInformation, p_onvif_device_list, 0, NULL);
    handleArray[3] = CreateThread(NULL, 0, getDeviceProfiles, p_onvif_device_list, 0, NULL);
    //handleArray[4] = CreateThread(NULL, 0, searchDevice, p_onvif_device_list, 0, NULL);

    WaitForMultipleObjects(4, handleArray, TRUE, INFINITE);

    free_device_list(&p_onvif_device_list);

    uninit_DLL();

    system("pause");

    return 0;
}

