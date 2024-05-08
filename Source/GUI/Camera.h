#pragma once
#include <JuceHeader.h> // for the camera device class
#include <iostream>

namespace HueShift {

class Camera : public juce::Component {
private:
    CameraDevice* camera = nullptr;
    Component* cameraViewer = nullptr;

public:
    Camera() {
       
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

    void SetCamera(CameraDevice* cam) {
        if (cam == nullptr){ 
            std::cout << "Camera could not be opened\n";
            return;
        }

        delete camera;
        delete cameraViewer;

        camera = cam;
        cameraViewer = camera->createViewerComponent();
        addAndMakeVisible(cameraViewer);
        resized();
    }
};

}
