#ifndef NEBULOSITY_GPHOTO_CAPTURE_H
#define NEBULOSITY_GPHOTO_CAPTURE_H
#include "GPhotoDiscovery.h"
#include <memory>
#include <string>
struct GPhotoCaptureResult {
    int code = 0;
    std::string error;
    std::string path;
};
class GPhotoCaptureSession {
public:
    GPhotoCaptureSession();
    ~GPhotoCaptureSession();
    GPhotoCaptureSession(const GPhotoCaptureSession&) = delete;
    GPhotoCaptureSession& operator=(const GPhotoCaptureSession&) = delete;
    GPhotoCaptureResult Connect(const GPhotoDeviceInfo& device);
    GPhotoCaptureResult Capture(const std::string& directory);
    void Cancel();
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
#endif
