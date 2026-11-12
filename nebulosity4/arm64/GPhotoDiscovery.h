#ifndef NEBULOSITY_GPHOTO_DISCOVERY_H
#define NEBULOSITY_GPHOTO_DISCOVERY_H
#include <string>
#include <vector>
struct GPhotoDeviceInfo {
    std::string model;
    std::string port;
};
struct GPhotoDiscoveryResult {
    std::vector<GPhotoDeviceInfo> devices;
    std::string error;
};
GPhotoDiscoveryResult FindGPhotoCameras();
#endif
