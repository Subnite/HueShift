#pragma once
#include <JuceHeader.h> // for the camera device class
#include <iostream>

class Camera : public juce::Component {
private:
    CameraDevice* camera = nullptr;
    Component* cameraViewer = nullptr;
public:
    Camera() {
        auto devices = CameraDevice::getAvailableDevices();
        
        int idx = 0;
        bool droidCamFound = false;

        std::cout << "Camera's:\n";
        for (int i = 0; i < devices.size(); i++) {
            auto s = devices[i];
            std::cout << s << "\n";
            // get integrated camera automatically as backup
            if (!droidCamFound && s.contains("Integrated")) {
                idx = i;
                std::cout << "Found integrated cam!\n";
            }
            // use droidcam if found
            else if (!droidCamFound && s.contains("DroidCam")) {
                idx = i;
                droidCamFound = true;
                std::cout << "Found DroidCam!\n";
            }
        }

        camera = CameraDevice::openDevice(
            idx,    // device idx
            0,      // min w
            0,      // min h
            1024,   // max w
            768,    // max h
            true    // high quality mode
        );
        
        if (camera != nullptr) {
            cameraViewer = camera->createViewerComponent();
            addAndMakeVisible(cameraViewer);
        } else {
            std::cout << "Camera couldn't be opened!\n\n";
        }

    }
    ~Camera() {
        delete camera;
        delete cameraViewer;
    }

    void resized() override {
        if (cameraViewer != nullptr){
            cameraViewer->setBounds(getLocalBounds());
        }
    }
};
