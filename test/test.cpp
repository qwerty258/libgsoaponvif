#include <OnvifOperation.h>
#include <iostream>
using namespace std;

typedef struct
{
    char IP[17];
    char username[50];
    char password[50];
}struct_IP_username_password;


int main(int argc, char* argv[])
{
    struct_IP_username_password IP_username_password[] =
    {
        "192.168.10.185", "admin", "12345",
        "192.168.10.141", "admin", "12345",
        "192.168.10.146", "admin", "Tolendata",
        "192.168.10.182", "admin", "Tolendata",
        "192.168.10.149", "admin", "Tolendata",
        "192.168.10.181", "admin", "Tolendata",
        "192.168.10.195", "admin", "12345",
        "192.168.10.142", "admin", "12345",
        "192.168.10.231", "admin", "Tolendata",
        "192.168.10.150", "admin", "Tolendata",
        "192.168.10.147", "admin", "12345"
        "192.168.10.222", "admin", "12345"
        "192.168.10.221", "admin", "12345"
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

    if(-1 == search_onvif_device(p_onvif_device_list, 1))
    {
        cout << "search device failed\n";
        system("pause");
        return -1;
    }

    cout << "\nsearch finished\n";
    system("pause");

    for(size_t j = 0; j < sizeof(IP_username_password) / sizeof(struct_IP_username_password); j++)
    {
        set_onvif_device_authorization_information(p_onvif_device_list, IP_username_password[j].IP, 555555, IP_username_password[j].username, IP_username_password[j].password);
    }

    for(size_t i = 0; i < p_onvif_device_list->number_of_onvif_devices; ++i)
    {
        cout << p_onvif_device_list->p_onvif_devices[i].IPv4 << ':' << p_onvif_device_list->p_onvif_devices[i].service_address_device_service.xaddr << endl;

        cout << p_onvif_device_list->p_onvif_devices[i].username << ':' << p_onvif_device_list->p_onvif_devices[i].password << endl;

        get_onvif_device_information(p_onvif_device_list, NULL, i);

        cout << "FirmwareVersion: " << p_onvif_device_list->p_onvif_devices[i].device_information.firmware_version << endl;
        cout << "HardwareId: " << p_onvif_device_list->p_onvif_devices[i].device_information.hardware_Id << endl;
        cout << "Manufacturer: " << p_onvif_device_list->p_onvif_devices[i].device_information.manufacturer << endl;
        cout << "Model: " << p_onvif_device_list->p_onvif_devices[i].device_information.model << endl;
        cout << "SerialNumber: " << p_onvif_device_list->p_onvif_devices[i].device_information.serial_number << endl;
        cout << "MAC Address: " << p_onvif_device_list->p_onvif_devices[i].device_information.MAC_address << endl;
        cout << endl;

        get_onvif_device_service_addresses(p_onvif_device_list, NULL, i);

        cout << "service addresses:\n";
        cout << p_onvif_device_list->p_onvif_devices[i].service_address_analytics.xaddr
            << ':'
            << p_onvif_device_list->p_onvif_devices[i].service_address_analytics.major_version
            << '.'
            << p_onvif_device_list->p_onvif_devices[i].service_address_analytics.minor_version
            << endl;
        cout << p_onvif_device_list->p_onvif_devices[i].service_address_deviceIO.xaddr
            << ':'
            << p_onvif_device_list->p_onvif_devices[i].service_address_deviceIO.major_version
            << '.'
            << p_onvif_device_list->p_onvif_devices[i].service_address_deviceIO.minor_version
            << endl;
        cout << p_onvif_device_list->p_onvif_devices[i].service_address_device_service.xaddr
            << ':'
            << p_onvif_device_list->p_onvif_devices[i].service_address_device_service.major_version
            << '.'
            << p_onvif_device_list->p_onvif_devices[i].service_address_device_service.minor_version
            << endl;
        cout << p_onvif_device_list->p_onvif_devices[i].service_address_events.xaddr
            << ':'
            << p_onvif_device_list->p_onvif_devices[i].service_address_events.major_version
            << '.'
            << p_onvif_device_list->p_onvif_devices[i].service_address_events.minor_version
            << endl;
        cout << p_onvif_device_list->p_onvif_devices[i].service_address_imaging.xaddr
            << ':'
            << p_onvif_device_list->p_onvif_devices[i].service_address_imaging.major_version
            << '.'
            << p_onvif_device_list->p_onvif_devices[i].service_address_imaging.minor_version
            << endl;
        cout << p_onvif_device_list->p_onvif_devices[i].service_address_media.xaddr
            << ':'
            << p_onvif_device_list->p_onvif_devices[i].service_address_media.major_version
            << '.'
            << p_onvif_device_list->p_onvif_devices[i].service_address_media.minor_version
            << endl;
        cout << p_onvif_device_list->p_onvif_devices[i].service_address_PTZ.xaddr
            << ':'
            << p_onvif_device_list->p_onvif_devices[i].service_address_PTZ.major_version
            << '.'
            << p_onvif_device_list->p_onvif_devices[i].service_address_PTZ.minor_version
            << endl;
        cout << p_onvif_device_list->p_onvif_devices[i].service_address_recording.xaddr
            << ':'
            << p_onvif_device_list->p_onvif_devices[i].service_address_recording.major_version
            << '.'
            << p_onvif_device_list->p_onvif_devices[i].service_address_recording.minor_version
            << endl;
        cout << p_onvif_device_list->p_onvif_devices[i].service_address_replay.xaddr
            << ':'
            << p_onvif_device_list->p_onvif_devices[i].service_address_replay.major_version
            << '.'
            << p_onvif_device_list->p_onvif_devices[i].service_address_replay.minor_version
            << endl;
        cout << p_onvif_device_list->p_onvif_devices[i].service_address_search_recording.xaddr
            << ':'
            << p_onvif_device_list->p_onvif_devices[i].service_address_search_recording.major_version
            << '.'
            << p_onvif_device_list->p_onvif_devices[i].service_address_search_recording.minor_version
            << endl;
        cout << p_onvif_device_list->p_onvif_devices[i].service_address_receiver.xaddr
            << ':'
            << p_onvif_device_list->p_onvif_devices[i].service_address_receiver.major_version
            << '.'
            << p_onvif_device_list->p_onvif_devices[i].service_address_receiver.minor_version
            << endl;
        cout << endl;

        get_onvif_ipc_profiles(p_onvif_device_list, NULL, i);

        for(size_t j = 0; j < p_onvif_device_list->p_onvif_devices[i].number_of_onvif_ipc_profiles; ++j)
        {
            cout << "Profile " << j + 1 << ": \n";
            cout << "Profile name: " << p_onvif_device_list->p_onvif_devices[i].p_onvif_ipc_profiles[j].name << endl;
            cout << "Resolution: "
                << "Width: "
                << p_onvif_device_list->p_onvif_devices[i].p_onvif_ipc_profiles[j].VideoEncoderConfiguration.Resolution.Width
                << " , Height: "
                << p_onvif_device_list->p_onvif_devices[i].p_onvif_ipc_profiles[j].VideoEncoderConfiguration.Resolution.Height
                << endl;
            cout << "Frame rate limit: "
                << p_onvif_device_list->p_onvif_devices[i].p_onvif_ipc_profiles[j].VideoEncoderConfiguration.RateControl.FrameRateLimit
                << endl;
            cout << "Encoding type: ";
            switch(p_onvif_device_list->p_onvif_devices[i].p_onvif_ipc_profiles[j].VideoEncoderConfiguration.encoding)
            {
                case H264:
                    cout << "H264" << endl;
                    break;
                case MPEG4:
                    cout << "MPEG4" << endl;
                    break;
                case JPEG:
                    cout << "JPEG" << endl;
                    break;
                default:
                    cout << "Unknown" << endl;
                    break;
            }
            cout << "URI: " << p_onvif_device_list->p_onvif_devices[i].p_onvif_ipc_profiles[j].MediaUri.URI << endl << endl;
        }

        get_onvif_nvr_receivers(p_onvif_device_list, NULL, i);

        for(size_t j = 0; j < p_onvif_device_list->p_onvif_devices[i].number_of_onvif_NVR_receivers; j++)
        {
            cout << "Receivers:\n";
            cout << p_onvif_device_list->p_onvif_devices[i].p_onvif_NVR_receivers[j].token
                << ':'
                << p_onvif_device_list->p_onvif_devices[i].p_onvif_NVR_receivers[j].configuration.media_URI
                << endl;
        }

        cout << "----------------------------------------------" << endl;
        system("pause");
        cout << endl;
    }

    cout << endl;
    cout << endl;
    cout << endl;

    cout << "search again\n";
    system("pause");

    if(-1 == search_onvif_device(p_onvif_device_list, 1))
    {
        cout << "search device failed\n";
        system("pause");
        return -1;
    }

    for(size_t i = 0; i < p_onvif_device_list->number_of_onvif_devices; ++i)
    {
        cout << p_onvif_device_list->p_onvif_devices[i].IPv4 << ':' << p_onvif_device_list->p_onvif_devices[i].service_address_device_service.xaddr << endl;

        cout << p_onvif_device_list->p_onvif_devices[i].username << ':' << p_onvif_device_list->p_onvif_devices[i].password << endl;

        cout << "FirmwareVersion: " << p_onvif_device_list->p_onvif_devices[i].device_information.firmware_version << endl;
        cout << "HardwareId: " << p_onvif_device_list->p_onvif_devices[i].device_information.hardware_Id << endl;
        cout << "Manufacturer: " << p_onvif_device_list->p_onvif_devices[i].device_information.manufacturer << endl;
        cout << "Model: " << p_onvif_device_list->p_onvif_devices[i].device_information.model << endl;
        cout << "SerialNumber: " << p_onvif_device_list->p_onvif_devices[i].device_information.serial_number << endl;
        cout << "MAC Address: " << p_onvif_device_list->p_onvif_devices[i].device_information.MAC_address << endl;

        for(size_t j = 0; j < p_onvif_device_list->p_onvif_devices[i].number_of_onvif_ipc_profiles; j++)
        {
            cout << "URI: " << p_onvif_device_list->p_onvif_devices[i].p_onvif_ipc_profiles[j].MediaUri.URI << endl;
        }

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