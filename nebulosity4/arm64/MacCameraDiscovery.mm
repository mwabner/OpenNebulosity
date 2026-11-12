// Native camera discovery for the ARM64 preview.
// The Apple discovery path observes capabilities without opening a session.
// Explicit test captures are handled separately through a native gPhoto2 session.
#import <Foundation/Foundation.h>
#import <ImageCaptureCore/ImageCaptureCore.h>

@interface NebCameraBrowserObserver : NSObject <ICDeviceBrowserDelegate>
@property(nonatomic) BOOL enumerated;
@end
@implementation NebCameraBrowserObserver
- (void)deviceBrowser:(ICDeviceBrowser*)browser didAddDevice:(ICDevice*)device moreComing:(BOOL)moreComing {}
- (void)deviceBrowser:(ICDeviceBrowser*)browser didRemoveDevice:(ICDevice*)device moreGoing:(BOOL)moreGoing {}
- (void)deviceBrowserDidEnumerateLocalDevices:(ICDeviceBrowser*)browser { self.enumerated = YES; }
@end

static NSString* CameraManufacturer(int vendor) {
    switch (vendor) {
        case 0x054c: return @"Sony";
        case 0x04b0: return @"Nikon";
        case 0x04a9: return @"Canon";
        default: return @"Other";
    }
}

static NSArray<NSDictionary*>* CameraSnapshot(ICDeviceBrowser* browser) {
    NSMutableArray<NSDictionary*>* result = [NSMutableArray array];
    for (ICDevice* device in [browser.devices copy]) {
        if (![device isKindOfClass:[ICCameraDevice class]]) continue;
        NSArray* capabilities = device.capabilities ?: @[];
        [result addObject:@{
            @"manufacturer": CameraManufacturer(device.usbVendorID),
            @"model": device.name ?: @"Unnamed camera",
            @"transport": device.transportType ?: @"Unknown",
            @"canTakePicture": @([capabilities containsObject:ICCameraDeviceCanTakePicture]),
            @"canSendPTP": @([capabilities containsObject:ICCameraDeviceCanAcceptPTPCommands])
        }];
    }
    return result;
}

static void StartCameraBrowser(ICDeviceBrowser* browser, NebCameraBrowserObserver* observer) {
    browser.delegate = observer;
    browser.browsedDeviceTypeMask = (ICDeviceTypeMask)(ICDeviceTypeMaskCamera | ICDeviceLocationTypeMaskLocal);
    [browser start];
}

#ifdef NEB_CAMERA_PROBE_MAIN
#include <cstdio>
int main() {
    @autoreleasepool {
        NebCameraBrowserObserver* observer = [NebCameraBrowserObserver new];
        ICDeviceBrowser* browser = [ICDeviceBrowser new];
        StartCameraBrowser(browser, observer);
        NSDate* deadline = [NSDate dateWithTimeIntervalSinceNow:4.0];
        while ([deadline timeIntervalSinceNow] > 0.0) {
            [[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.1]];
        }
        NSDictionary* report = @{
            @"backend": @"ImageCaptureCore",
            @"scope": @"local-camera-discovery-only",
            @"enumerationCompleted": @(observer.enumerated),
            @"cameras": CameraSnapshot(browser)
        };
        NSError* error = nil;
        NSData* data = [NSJSONSerialization dataWithJSONObject:report options:NSJSONWritingPrettyPrinted error:&error];
        browser.delegate = nil;
        [browser stop];
        if (!data) return 1;
        fwrite(data.bytes, 1, data.length, stdout);
        fputc('\n', stdout);
    }
    return 0;
}
#else
#include <wx/wx.h>
#include <wx/listctrl.h>
#include <wx/timer.h>
#include <wx/stopwatch.h>
#include <wx/dirdlg.h>
#include <wx/stdpaths.h>
#include "MacCameraDiscovery.h"
#include "GPhotoDiscovery.h"
#include "GPhotoCapture.h"
#include <gphoto2/gphoto2-result.h>
#include <future>
#include <chrono>
#include <libproc.h>
#include <sys/proc_info.h>
#include <signal.h>
#include <unistd.h>

static wxString CameraText(NSString* value) {
    return value ? wxString::FromUTF8(value.UTF8String) : wxString();
}

// Invoked only after an explicit user choice to release the camera's USB connection.
static void ReleaseCameraImporter() {
    int capacity = proc_listallpids(nullptr, 0) + 64;
    if (capacity <= 64) return;
    std::vector<pid_t> pids(capacity);
    int count = proc_listallpids(pids.data(), static_cast<int>(pids.size() * sizeof(pid_t)));
    for (int i=0; i<count && i<capacity; ++i) {
        proc_bsdinfo info{};
        if (proc_pidinfo(pids[i], PROC_PIDTBSDINFO, 0, &info, sizeof(info)) != sizeof(info)) continue;
        if (info.pbi_uid != getuid()) continue;
        const std::string name(info.pbi_comm);
        if (name != "ptpcamerad" && name != "PTPCamera") continue;
        kill(pids[i], SIGTERM);
        usleep(200000);
        if (kill(pids[i], 0) == 0) kill(pids[i], SIGKILL);
    }
}

class MacCameraDiscoveryDialog : public wxDialog {
public:
    explicit MacCameraDiscoveryDialog(wxWindow* parent)
        : wxDialog(parent, wxID_ANY, "Sony / Nikon / Canon camera test", wxDefaultPosition, wxSize(820, 370),
                   wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER), timer_(this), session_(std::make_shared<GPhotoCaptureSession>()) {
        auto* layout = new wxBoxSizer(wxVERTICAL);
        layout->Add(new wxStaticText(this, wxID_ANY,
            "Connect the camera by USB in PC Remote mode. Set exposure and file format on the camera.\n"
            "This experimental test downloads a single image. Bulb exposure is not yet available."),
            0, wxEXPAND | wxALL, 12);
        list_ = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT);
        list_->InsertColumn(0, "Manufacturer", wxLIST_FORMAT_LEFT, 100);
        list_->InsertColumn(1, "Camera", wxLIST_FORMAT_LEFT, 260);
        list_->InsertColumn(2, "Connection", wxLIST_FORMAT_LEFT, 110);
        list_->InsertColumn(3, "Shutter capability", wxLIST_FORMAT_LEFT, 130);
        list_->InsertColumn(4, "PTP capability", wxLIST_FORMAT_LEFT, 110);
        layout->Add(list_, 1, wxEXPAND | wxLEFT | wxRIGHT, 12);
        status_ = new wxStaticText(this, wxID_ANY, "Searching for cameras...");
        layout->Add(status_, 0, wxEXPAND | wxALL, 12);
        auto* buttons = new wxBoxSizer(wxHORIZONTAL);
        refresh_ = new wxButton(this, wxID_ANY, "Refresh");
        connect_ = new wxButton(this, wxID_ANY, "Connect camera");
        capture_ = new wxButton(this, wxID_ANY, "Take test image");
        cancel_ = new wxButton(this, wxID_ANY, "Cancel operation");
        close_ = new wxButton(this, wxID_CLOSE, "Close");
        for (auto* button : {refresh_, connect_, capture_, cancel_, close_}) buttons->Add(button, 0, wxLEFT, 8);
        layout->Add(buttons, 0, wxALIGN_RIGHT | wxALL, 12);
        SetSizer(layout);SetMinSize(wxSize(820, 370));
        observer_ = [NebCameraBrowserObserver new];browser_ = [ICDeviceBrowser new];
        StartCameraBrowser(browser_, observer_);
        Bind(wxEVT_TIMER, &MacCameraDiscoveryDialog::RefreshDevices, this);
        refresh_->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { StartDiscovery(); });
        connect_->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { ChooseCamera(); });
        capture_->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { CaptureImage(); });
        cancel_->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { session_->Cancel(); message_="Cancelling camera operation..."; });
        close_->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { if(!busy_)EndModal(wxID_CLOSE); });
        Bind(wxEVT_CLOSE_WINDOW, [this](wxCloseEvent& event) {
            if(busy_) { session_->Cancel(); message_="Cancelling camera operation...";event.Veto(); }
            else EndModal(wxID_CLOSE);
        });
        StartDiscovery();timer_.Start(300);UpdateButtons();
    }
    ~MacCameraDiscoveryDialog() override { timer_.Stop();browser_.delegate=nil;[browser_ stop]; }
    wxString CapturedFile() const { return capturedFile_; }
private:
    void UpdateButtons() {
        refresh_->Enable(!busy_ && !connected_ && !gphoto_.valid());
        connect_->Enable(!busy_ && !connected_ && !gphoto_.valid() && !devices_.devices.empty());
        capture_->Enable(!busy_ && connected_);cancel_->Enable(busy_);close_->Enable(!busy_);
    }
    void StartDiscovery() {
        if(gphoto_.valid() || busy_ || connected_)return;
        message_.clear();gphotoDone_=false;
        gphoto_=std::async(std::launch::async, FindGPhotoCameras);UpdateButtons();
    }
    void ChooseCamera() {
        size_t selected=0;
        if(devices_.devices.size()>1) {
            wxArrayString names;
            for(const auto& item:devices_.devices) names.Add(wxString::FromUTF8(item.model.c_str())+" ("+wxString::FromUTF8(item.port.c_str())+")");
            int result=wxGetSingleChoiceIndex("Choose a USB camera", "Camera", names, this);
            if(result==wxNOT_FOUND)return;
            selected=static_cast<size_t>(result);
        }
        if(devices_.devices.empty())return;
        selected_=devices_.devices[selected];
        browser_.delegate=nil;[browser_ stop];
        releaseOffered_=false;StartConnection();
    }
    void StartConnection() {
        busy_=true;connecting_=true;message_="Connecting to camera...";
        auto session=session_;auto camera=selected_;
        operation_=std::async(std::launch::async,[session,camera]{return session->Connect(camera);});UpdateButtons();
    }
    void CaptureImage() {
        wxDirDialog folder(this,"Choose a folder for the test image",wxStandardPaths::Get().GetDocumentsDir(),wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
        if(folder.ShowModal()!=wxID_OK)return;
        std::string directory=folder.GetPath().ToUTF8().data();
        busy_=true;connecting_=false;message_="Capturing and downloading image...";
        auto session=session_;
        operation_=std::async(std::launch::async,[session,directory]{return session->Capture(directory);});UpdateButtons();
    }
    void CheckOperation() {
        if(!operation_.valid() || operation_.wait_for(std::chrono::seconds(0))!=std::future_status::ready)return;
        GPhotoCaptureResult result;
        try { result=operation_.get(); }
        catch(const std::exception& error){result.error=error.what();result.code=GP_ERROR;}
        busy_=false;
        if(!result.error.empty()) {
            message_=wxString::FromUTF8(result.error.c_str());
            if(connecting_ && result.code==GP_ERROR_IO_USB_CLAIM && !releaseOffered_) {
                releaseOffered_=true;
                if(wxMessageBox("macOS image import currently holds the camera. Release it for OpenNebulosity?\n\nAny active camera import may be interrupted.","Release camera connection",wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION,this)==wxYES) {
                    ReleaseCameraImporter();StartConnection();return;
                }
            }
        } else if(connecting_) { connected_=true;message_="Camera connected. Use its controls to select exposure and file format."; }
        else { capturedFile_=wxString::FromUTF8(result.path.c_str());message_="Saved: "+capturedFile_+"\nClose this window to open the image."; }
        UpdateButtons();
    }
    void RefreshDevices(wxTimerEvent&) {
        CheckOperation();
        @autoreleasepool {
            if(gphoto_.valid() && gphoto_.wait_for(std::chrono::seconds(0))==std::future_status::ready) {
                try { devices_=gphoto_.get(); }catch(const std::exception& error){devices_.error=error.what();}
                gphotoDone_=true;UpdateButtons();
            }
            NSMutableArray<NSDictionary*>* snapshot=[CameraSnapshot(browser_) mutableCopy];
            for(const auto& camera:devices_.devices) {
                NSString* model=[NSString stringWithUTF8String:camera.model.c_str()];
                NSString* manufacturer=[model hasPrefix:@"Sony"]?@"Sony":[model hasPrefix:@"Nikon"]?@"Nikon":[model hasPrefix:@"Canon"]?@"Canon":@"Other";
                [snapshot addObject:@{@"manufacturer":manufacturer,@"model":model,@"transport":@"gPhoto2 USB",@"canTakePicture":@NO,@"canSendPTP":@YES}];
            }
            NSString* signature=snapshot.description;
            if(![signature isEqualToString:lastSnapshot_]) {
                lastSnapshot_=[signature copy];list_->DeleteAllItems();
                for(NSDictionary* item in snapshot) {
                    long row=list_->InsertItem(list_->GetItemCount(),CameraText(item[@"manufacturer"]));
                    list_->SetItem(row,1,CameraText(item[@"model"]));list_->SetItem(row,2,CameraText(item[@"transport"]));
                    list_->SetItem(row,3,[item[@"transport"] isEqualToString:@"gPhoto2 USB"]?"Driver-specific":[item[@"canTakePicture"] boolValue]?"Reported":"Not reported");
                    list_->SetItem(row,4,[item[@"canSendPTP"] boolValue]?"Reported":"Not reported");
                }
            }
            if(!message_.empty())status_->SetLabel(message_);
            else if(!devices_.error.empty())status_->SetLabel("USB camera discovery failed: "+wxString::FromUTF8(devices_.error.c_str()));
            else if(snapshot.count)status_->SetLabel(wxString::Format("%lu camera connection(s) detected.",(unsigned long)snapshot.count));
            else if(observer_.enumerated && gphotoDone_)status_->SetLabel("No camera detected. Check USB mode and close other camera-control applications.");
            else if(searchTime_.Time()>8000)status_->SetLabel("Camera search has not completed. Reconnect the camera and refresh.");
        }
    }
    std::future<GPhotoDiscoveryResult> gphoto_;
    GPhotoDiscoveryResult devices_;
    std::shared_ptr<GPhotoCaptureSession> session_;
    std::future<GPhotoCaptureResult> operation_;
    GPhotoDeviceInfo selected_;
    bool gphotoDone_=false,busy_=false,connecting_=false,connected_=false,releaseOffered_=false;
    wxString message_,capturedFile_;
    wxListCtrl* list_=nullptr;wxStaticText* status_=nullptr;
    wxButton *refresh_=nullptr,*connect_=nullptr,*capture_=nullptr,*cancel_=nullptr,*close_=nullptr;
    wxTimer timer_;wxStopWatch searchTime_;
    ICDeviceBrowser* __strong browser_;
    NebCameraBrowserObserver* __strong observer_;
    NSString* __strong lastSnapshot_;
};
wxString ShowMacCameraDiscovery(wxWindow* parent) {
    MacCameraDiscoveryDialog dialog(parent);dialog.ShowModal();return dialog.CapturedFile();
}
#endif
