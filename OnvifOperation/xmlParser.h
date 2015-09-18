#pragma once

#include "OnvifOperation.h"

#ifdef __cplusplus
extern "C" {
#endif // _cplusplus

    void parseDiscoveredDeviceXML(onvif_device_list* p_onvif_device_list, void* receivedDataList);

    void parseGetServicesResponse(onvif_device* p_onvif_device, void* receivedDataList);

#ifdef __cplusplus
}
#endif // _cplusplus