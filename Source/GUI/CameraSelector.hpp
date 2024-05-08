#pragma once
#include <JuceHeader.h>
#include "Camera.h"

namespace HueShift {

class CameraSelector : public juce::ComboBox {
private:
	HueShift::Camera* camera;

	void ResetCameraOptions() {
		clear();

		auto devices = CameraDevice::getAvailableDevices();

		addItemList(devices, 1); // ID indexes start at 1, so not 0 :/

		onChange = [this](){
			int idx = getSelectedItemIndex();
			auto name = getItemText(idx);

			auto devices = CameraDevice::getAvailableDevices();
			idx = devices.indexOf(name);

			auto newCam = CameraDevice::openDevice(
				idx,    // device idx
				0,      // min w
				0,      // min h
				8000, // 1024,   // max w
				8000, // 768,    // max h
				true    // high quality mode
			);

			camera->SetCamera(newCam);
		};
	}

public:
	CameraSelector(HueShift::Camera* camera)
	: camera(camera)
	{
		ResetCameraOptions();

	}

};

}
