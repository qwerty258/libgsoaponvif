// test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <OnvifOperation.h>
#pragma comment(lib,"OnvifOperation.lib")
#include <iostream>
using namespace std;

typedef struct
{
    char IP[17];
    char username[50];
    char password[50];
}struct_IP_username_password;


int _tmain(int argc, _TCHAR* argv[])
{
    struct_IP_username_password IP_username_password[8] =
    {
        "192.168.10.185", "admin", "12345",
        "192.168.10.141", "admin", "12345",
        "192.168.10.146", "admin", "12345",
        "192.168.10.182", "admin", "Tolendata",
        "192.168.10.149", "admin", "Tolendata",
        "192.168.10.181", "admin", "Tolendata",
        "192.168.10.195", "admin", "12345",
        "192.168.10.142", "admin", "12345"
    };

    onvif_device_list* p_onvif_device_list = malloc_device_list();

    if(-1 == init_DLL())
    {
        cout << "initDll failed\n";
        system("pause");
        return -1;
    }

    cout << "\nsearch device\n\n";
    system("pause");

    if(-1 == search_ONVIF_device(p_onvif_device_list, 1))
    {
        cout << "search device failed\n";
        system("pause");
        return -1;
    }

    for(size_t i = 0; i < p_onvif_device_list->number_of_onvif_device; ++i)
    {
        cout << p_onvif_device_list->p_onvif_device[i].IPv4 << ':' << p_onvif_device_list->p_onvif_device[i].service_address_device_service.xaddr << endl;

        for(size_t j = 0; j < 8; j++)
        {
            if(0 == strncmp(p_onvif_device_list->p_onvif_device[i].IPv4, IP_username_password[j].IP, 17))
            {
                strncpy(p_onvif_device_list->p_onvif_device[i].username, IP_username_password[j].username, 50);
                strncpy(p_onvif_device_list->p_onvif_device[i].password, IP_username_password[j].password, 50);
            }
        }

        cout << p_onvif_device_list->p_onvif_device[i].username << ':' << p_onvif_device_list->p_onvif_device[i].password << endl;

        get_onvif_device_information(p_onvif_device_list, NULL, i);

        cout << "FirmwareVersion: " << p_onvif_device_list->p_onvif_device[i].device_information.firmware_version << endl;
        cout << "HardwareId: " << p_onvif_device_list->p_onvif_device[i].device_information.hardware_Id << endl;
        cout << "Manufacturer: " << p_onvif_device_list->p_onvif_device[i].device_information.manufacturer << endl;
        cout << "Model: " << p_onvif_device_list->p_onvif_device[i].device_information.model << endl;
        cout << "SerialNumber: " << p_onvif_device_list->p_onvif_device[i].device_information.serial_number << endl;
        cout << endl;

        get_onvif_device_service_address(p_onvif_device_list, NULL, i);

        cout << "service addresses:\n";
        cout << p_onvif_device_list->p_onvif_device[i].service_address_analytics.xaddr
            << ':'
            << p_onvif_device_list->p_onvif_device[i].service_address_analytics.major_version
            << '.'
            << p_onvif_device_list->p_onvif_device[i].service_address_analytics.minor_version
            << endl;
        cout << p_onvif_device_list->p_onvif_device[i].service_address_deviceIO.xaddr
            << ':'
            << p_onvif_device_list->p_onvif_device[i].service_address_deviceIO.major_version
            << '.'
            << p_onvif_device_list->p_onvif_device[i].service_address_deviceIO.minor_version
            << endl;
        cout << p_onvif_device_list->p_onvif_device[i].service_address_device_service.xaddr
            << ':'
            << p_onvif_device_list->p_onvif_device[i].service_address_device_service.major_version
            << '.'
            << p_onvif_device_list->p_onvif_device[i].service_address_device_service.minor_version
            << endl;
        cout << p_onvif_device_list->p_onvif_device[i].service_address_events.xaddr
            << ':'
            << p_onvif_device_list->p_onvif_device[i].service_address_events.major_version
            << '.'
            << p_onvif_device_list->p_onvif_device[i].service_address_events.minor_version
            << endl;
        cout << p_onvif_device_list->p_onvif_device[i].service_address_imaging.xaddr
            << ':'
            << p_onvif_device_list->p_onvif_device[i].service_address_imaging.major_version
            << '.'
            << p_onvif_device_list->p_onvif_device[i].service_address_imaging.minor_version
            << endl;
        cout << p_onvif_device_list->p_onvif_device[i].service_address_media.xaddr
            << ':'
            << p_onvif_device_list->p_onvif_device[i].service_address_media.major_version
            << '.'
            << p_onvif_device_list->p_onvif_device[i].service_address_media.minor_version
            << endl;
        cout << p_onvif_device_list->p_onvif_device[i].service_address_PTZ.xaddr
            << ':'
            << p_onvif_device_list->p_onvif_device[i].service_address_PTZ.major_version
            << '.'
            << p_onvif_device_list->p_onvif_device[i].service_address_PTZ.minor_version
            << endl;
        cout << p_onvif_device_list->p_onvif_device[i].service_address_recording.xaddr
            << ':'
            << p_onvif_device_list->p_onvif_device[i].service_address_recording.major_version
            << '.'
            << p_onvif_device_list->p_onvif_device[i].service_address_recording.minor_version
            << endl;
        cout << p_onvif_device_list->p_onvif_device[i].service_address_replay.xaddr
            << ':'
            << p_onvif_device_list->p_onvif_device[i].service_address_replay.major_version
            << '.'
            << p_onvif_device_list->p_onvif_device[i].service_address_replay.minor_version
            << endl;
        cout << p_onvif_device_list->p_onvif_device[i].service_address_search_recording.xaddr
            << ':'
            << p_onvif_device_list->p_onvif_device[i].service_address_search_recording.major_version
            << '.'
            << p_onvif_device_list->p_onvif_device[i].service_address_search_recording.minor_version
            << endl;
        cout << "----------------------------------------------" << endl;
        cout << endl;
    }

    cout << endl;
    cout << endl;
    cout << endl;

    cout << "search again\n";
    system("pause");

    if(-1 == search_ONVIF_device(p_onvif_device_list, 1))
    {
        cout << "search device failed\n";
        system("pause");
        return -1;
    }

    for(size_t i = 0; i < p_onvif_device_list->number_of_onvif_device; ++i)
    {
        cout << p_onvif_device_list->p_onvif_device[i].IPv4 << ':' << p_onvif_device_list->p_onvif_device[i].service_address_device_service.xaddr << endl;

        cout << p_onvif_device_list->p_onvif_device[i].username << ':' << p_onvif_device_list->p_onvif_device[i].password << endl;

        cout << "FirmwareVersion: " << p_onvif_device_list->p_onvif_device[i].device_information.firmware_version << endl;

        cout << "HardwareId: " << p_onvif_device_list->p_onvif_device[i].device_information.hardware_Id << endl;

        cout << "Manufacturer: " << p_onvif_device_list->p_onvif_device[i].device_information.manufacturer << endl;

        cout << "Model: " << p_onvif_device_list->p_onvif_device[i].device_information.model << endl;

        cout << "SerialNumber: " << p_onvif_device_list->p_onvif_device[i].device_information.serial_number << endl;

        cout << "----------------------------------------------" << endl;

        cout << endl;
    }


    if(-1 == uninit_DLL())
    {
        cout << "uninit_DLL failed\n";
        system("pause");
        return -1;
    }

    free_device_list(&p_onvif_device_list);

    system("pause");
    return 0;
}

