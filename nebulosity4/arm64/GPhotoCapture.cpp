// Shared native libgphoto2 session. The connection stays open between operations.
// Camera settings and existing camera files are preserved.
#include "GPhotoCapture.h"
#include <gphoto2/gphoto2-camera.h>
#include <gphoto2/gphoto2-result.h>
#include <atomic>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>
#include <algorithm>
#include <cctype>

struct GPhotoCaptureSession::Impl {
    GPContext* context = gp_context_new();
    Camera* camera = nullptr;
    bool connected = false;
    std::atomic<bool> cancelled{false};
    ~Impl() { Close(); if(context)gp_context_unref(context); }
    void Close() {
        if(camera) { if(connected)gp_camera_exit(camera,context); gp_camera_unref(camera); }
        camera=nullptr;connected=false;
    }
    GPhotoCaptureResult Failure(int code, const std::string& message) {
        return {code, message + (code < 0 ? std::string(": ") + gp_result_as_string(code) : ""), ""};
    }
};
GPhotoCaptureSession::GPhotoCaptureSession():impl_(new Impl) {
    if(impl_->context)gp_context_set_cancel_func(impl_->context, [](GPContext*, void* data) {
        return static_cast<Impl*>(data)->cancelled.load() ? GP_CONTEXT_FEEDBACK_CANCEL : GP_CONTEXT_FEEDBACK_OK;
    },impl_.get());
}
GPhotoCaptureSession::~GPhotoCaptureSession() = default;
void GPhotoCaptureSession::Cancel() { impl_->cancelled=true; }
GPhotoCaptureResult GPhotoCaptureSession::Connect(const GPhotoDeviceInfo& device) {
    auto& p=*impl_;p.Close();p.cancelled=false;
    if(!p.context)return p.Failure(GP_ERROR_NO_MEMORY,"Could not create camera context");
    int rc=gp_camera_new(&p.camera);if(rc<GP_OK)return p.Failure(rc,"Could not create camera session");
    CameraAbilitiesList* abilities=nullptr;GPPortInfoList* ports=nullptr;
    rc=gp_abilities_list_new(&abilities);
    if(rc>=GP_OK)rc=gp_abilities_list_load(abilities,p.context);
    int index=rc>=GP_OK?gp_abilities_list_lookup_model(abilities,device.model.c_str()):rc;
    CameraAbilities ability{};
    rc=index>=0?gp_abilities_list_get_abilities(abilities,index,&ability):index;
    if(rc>=GP_OK && !(ability.operations&GP_OPERATION_CAPTURE_IMAGE))rc=GP_ERROR_NOT_SUPPORTED;
    if(rc>=GP_OK)rc=gp_camera_set_abilities(p.camera,ability);
    if(rc>=GP_OK)rc=gp_port_info_list_new(&ports);
    if(rc>=GP_OK)rc=gp_port_info_list_load(ports);
    index=rc>=GP_OK?gp_port_info_list_lookup_path(ports,device.port.c_str()):rc;
    GPPortInfo port=nullptr;
    rc=index>=0?gp_port_info_list_get_info(ports,index,&port):index;
    if(rc>=GP_OK)rc=gp_camera_set_port_info(p.camera,port);
    if(abilities)gp_abilities_list_free(abilities);
    if(ports)gp_port_info_list_free(ports);
    if(rc>=GP_OK)rc=gp_camera_init(p.camera,p.context);
    if(rc<GP_OK) { p.Close();return p.Failure(rc,"Could not open the camera"); }
    p.connected=true;
    CameraWidget* initialConfig=nullptr;
    if(gp_camera_get_config(p.camera,&initialConfig,p.context)>=GP_OK && initialConfig)gp_widget_free(initialConfig);
    return {};
}
GPhotoCaptureResult GPhotoCaptureSession::Capture(const std::string& directory) {
    auto& p=*impl_;p.cancelled=false;
    if(!p.connected)return p.Failure(GP_ERROR_BAD_PARAMETERS,"Connect a camera first");
    // Use the camera's settings; refuse an unbounded Bulb exposure in this test tool.
    CameraWidget* config=nullptr;CameraWidget* shutter=nullptr;
    int rc=gp_camera_get_config(p.camera,&config,p.context);
    if(rc<GP_OK)return p.Failure(rc,"Could not read the current exposure");
    rc=gp_widget_get_child_by_name(config,"shutterspeed",&shutter);
    const char* value=nullptr;
    if(rc>=GP_OK)rc=gp_widget_get_value(shutter,&value);
    std::string exposure=value?value:"";
    gp_widget_free(config);
    std::transform(exposure.begin(),exposure.end(),exposure.begin(),[](unsigned char c){return std::tolower(c);});
    if(rc<GP_OK || exposure.empty() || exposure.find("bulb")!=std::string::npos || exposure=="time")
        return p.Failure(GP_ERROR_NOT_SUPPORTED,"Select a timed exposure on the camera; Bulb capture is not implemented in this test tool");
    struct stat directoryInfo{};
    if(stat(directory.c_str(),&directoryInfo)!=0 || !S_ISDIR(directoryInfo.st_mode))return p.Failure(GP_ERROR_BAD_PARAMETERS,"Select an existing output folder");
    CameraFilePath cameraPath{};
    rc=gp_camera_capture(p.camera,GP_CAPTURE_IMAGE,&cameraPath,p.context);
    if(rc<GP_OK)return p.Failure(rc,"Camera capture failed");
    CameraFile* file=nullptr;rc=gp_file_new(&file);
    if(rc>=GP_OK)rc=gp_camera_file_get(p.camera,cameraPath.folder,cameraPath.name,GP_FILE_TYPE_NORMAL,file,p.context);
    if(rc<GP_OK) { if(file)gp_file_unref(file);return p.Failure(rc,"Image download failed"); }
    std::string name(cameraPath.name);
    auto dot=name.find_last_of('.');
    std::string extension=dot==std::string::npos ? "" : name.substr(dot);
    if(extension.empty() || extension.size()>10 || extension.find_first_of("/\\")!=std::string::npos)extension=".image";
    std::string pattern=directory+(directory.back()=='/'?"":"/")+"OpenNebulosity-test-XXXXXX"+extension;
    std::vector<char> path(pattern.begin(),pattern.end());path.push_back('\0');
    int descriptor=mkstemps(path.data(),static_cast<int>(extension.size()));
    if(descriptor<0){gp_file_unref(file);return p.Failure(GP_ERROR_IO,"Could not create a new output file");}
    close(descriptor);
    rc=gp_file_save(file,path.data());gp_file_unref(file);
    if(rc<GP_OK) { unlink(path.data());return p.Failure(rc,"Could not save the downloaded image"); }
    // Deliberately no gp_camera_file_delete(): originals stay on the camera.
    return {GP_OK,"",path.data()};
}
