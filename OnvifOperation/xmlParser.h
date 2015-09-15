#pragma once

#include "OnvifOperation.h"

#ifdef __cplusplus
extern "C" {
#endif // _cplusplus

    void parseDiscoveredDeviceXML(onvif_device_list* p_onvif_device_list, void* receivedDataList);

#ifdef __cplusplus
}
#endif // _cplusplus