#ifndef CAMERA_H
#define CAMERA_H

#include "esp_camera.h"


void CameraSetup();
camera_fb_t* CameraCapture();
void CameraRelease(camera_fb_t *fb);
void CameraFlash(int on);
void CameraDeinit();

#endif