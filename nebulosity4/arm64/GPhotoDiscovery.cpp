// Read-only USB discovery: no camera session, capture or media access.
#include "GPhotoDiscovery.h"
#include <gphoto2/gphoto2-camera.h>
#include <gphoto2/gphoto2-result.h>

GPhotoDiscoveryResult FindGPhotoCameras() {
    GPhotoDiscoveryResult result;
    GPContext* context = gp_context_new();
    CameraList* list = nullptr;
    int status = gp_list_new(&list);
    if (status < GP_OK || !context) {
        result.error = "Could not initialize camera discovery.";
    } else {
        status = gp_camera_autodetect(list, context);
        if (status < GP_OK) result.error = gp_result_as_string(status);
        else for (int i = 0; i < gp_list_count(list); ++i) {
            const char* model = nullptr;
            const char* port = nullptr;
            if (gp_list_get_name(list, i, &model) >= GP_OK &&
                gp_list_get_value(list, i, &port) >= GP_OK && model && port) {
                result.devices.push_back({model, port});
            }
        }
    }
    if (list) gp_list_free(list);
    if (context) gp_context_unref(context);
    return result;
}

#ifdef NEB_GPHOTO_PROBE_MAIN
#include <cstdio>
int main() {
    auto result = FindGPhotoCameras();
    if (!result.error.empty()) { fprintf(stderr, "%s\n", result.error.c_str()); return 1; }
    printf("Detected cameras: %zu\n", result.devices.size());
    for (const auto& camera : result.devices) printf("%s\t%s\n", camera.model.c_str(), camera.port.c_str());
    CameraAbilitiesList* abilities = nullptr;
    GPContext* context = gp_context_new();
    int status = gp_abilities_list_new(&abilities);
    if (status >= GP_OK) status = gp_abilities_list_load(abilities, context);
    int index = status >= GP_OK ? gp_abilities_list_lookup_model(abilities, "Sony Alpha-A6000 (Control)") : status;
    CameraAbilities camera{};
    if (index >= 0) status = gp_abilities_list_get_abilities(abilities, index, &camera);
    else status = index;
    printf("Alpha 6000 Control USB identity: %04x:%04x\n", camera.usb_vendor, camera.usb_product);
    bool capture = status >= GP_OK && (camera.operations & GP_OPERATION_CAPTURE_IMAGE);
    bool configure = status >= GP_OK && (camera.operations & GP_OPERATION_CONFIG);
    printf("Installed Alpha 6000 driver: capture=%s configuration=%s\n", capture ? "yes" : "no", configure ? "yes" : "no");
    if (abilities) gp_abilities_list_free(abilities);
    if (context) gp_context_unref(context);
    return capture && configure ? 0 : 2;
}
#endif
