#include "../GPhotoCapture.h"
#include <cassert>
#include <cstdio>
int main(){
 GPhotoCaptureSession session;
 auto result=session.Capture(".");assert(result.code<0 && !result.error.empty() && result.path.empty());
 result=session.Connect({"nonexistent-test-camera","nonexistent-test-port"});
 assert(result.code<0 && !result.error.empty());
 result=session.Capture(".");assert(result.code<0 && result.path.empty());
 session.Cancel();
 puts("Capture without connection, failed connection cleanup and cancellation: PASS");
}
