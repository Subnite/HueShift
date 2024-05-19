#pragma once
#include <JuceHeader.h>
#include "Camera.h"
#include <vector>

namespace HueShift{


class CameraGrid : public juce::Component, public juce::Timer {
private:
	const HueShift::Camera& camera;
	HueShiftProcessor& audioProcessor;
	juce::Image currentSnapshot;
	std::vector<std::vector<juce::Colour>> snapshotOutput{}; // row -> column
	
	unsigned int samplePoints = 13, widthDivision = 2, heightDivision = 1;
	
	// samplePoints is how many pixels to check per section, width division is how many sections you want in the x axis, same for height.
	void CalculateGridOutput(const juce::Image& img, const unsigned int& samplePointsPerSection, const unsigned int& widthDivision, const unsigned int& heightDivision) {
		int width = img.getWidth();
		int height = img.getHeight();

		int pixelsPerWidth = width / widthDivision;
		int pixelsPerHeight = height / heightDivision;

		snapshotOutput.clear();
		snapshotOutput.reserve(heightDivision);
		for (int i = 0; i < heightDivision; i++) {
			std::vector<juce::Colour> vec{};
			vec.resize(widthDivision);
			snapshotOutput.push_back(vec);
		}

		for (int h = 0; h < heightDivision; h++){
			for (int w = 0; w < widthDivision; w++){
				int r{}, g{}, b{};
				const int x = w*pixelsPerWidth;
				const int y = h*pixelsPerHeight;
				const int sectionPixelAmount = pixelsPerWidth * pixelsPerHeight;
				const int addPerSample = sectionPixelAmount / samplePointsPerSection;

				for (int sample = 0; sample < samplePointsPerSection; sample++){
					if (sample > sectionPixelAmount) break;
					
					int addX = (sample * addPerSample) % pixelsPerWidth;
					int addY = (sample * addPerSample) / pixelsPerHeight;

					auto pix = img.getPixelAt(x+addX, y+addY);
					r += pix.getRed();
					g += pix.getGreen();
					b += pix.getBlue();
				}

				r /= samplePointsPerSection;
				g /= samplePointsPerSection;
				b /= samplePointsPerSection;

				snapshotOutput[h][w] = juce::Colour(r, g, b);
			}
		}
	}
	
	void timerCallback() override {
		UpdateGrid();

		// copy over the output to the processor
		const std::lock_guard<std::mutex> lock(audioProcessor.colourDataGuard);
		audioProcessor.colourData = GetIndexBasedColours();
	}

	void paint(Graphics &g) override {
		// g.drawImage(currentSnapshot, getLocalBounds().toFloat(), RectanglePlacement::onlyReduceInSize);
		
		auto bounds = getLocalBounds().toFloat();
		float xToYRelation = currentSnapshot.getWidth() / (currentSnapshot.getHeight()*1.f);
		float boundRatio = bounds.getWidth() / bounds.getHeight();
		
		if (boundRatio > xToYRelation) { 
			bounds.setWidth(bounds.getHeight() * xToYRelation );
			bounds.setX((getLocalBounds().getWidth() - bounds.getWidth()) * 0.5f);
		} else {
			bounds.setHeight(bounds.getWidth() / xToYRelation);
			bounds.setY((getLocalBounds().getHeight() - bounds.getHeight()) * 0.5f);
		}

		const float widthPerSection = bounds.getWidth() / widthDivision;
		const float heightPerSection = bounds.getHeight() / heightDivision;

		for (int h = 0; h < heightDivision; h++) {
			if (h+1 > snapshotOutput.size()) break;

			auto heightBounds = bounds.removeFromTop(heightPerSection);
			for (int w = 0; w < widthDivision; w++) {
				if (w+1 > snapshotOutput[h].size()) break;
				auto sectionBounds = heightBounds.removeFromLeft(widthPerSection);

				g.setColour(snapshotOutput[h][w]);
				g.fillRect(sectionBounds);
			}
		}
	}

	void mouseUp(const MouseEvent &event) override {
		UpdateGrid();	
	}

public:
	CameraGrid(const HueShift::Camera& camera, HueShiftProcessor& processor, const int& snapshotHz)
	: camera(camera), audioProcessor(processor)
	{
		startTimerHz(snapshotHz);
	}

	~CameraGrid() {
		stopTimer();
	}


	void UpdateGrid() {
		auto onPic = [this](const juce::Image& img){
			currentSnapshot = img;
			CalculateGridOutput(
				currentSnapshot,
				samplePoints, widthDivision, heightDivision
			);
			repaint();
		};

		camera.GetSnapshot(onPic);
	}

	void SetGridSettings(unsigned int samplePoints, unsigned int widthDivision, unsigned int heightDivision){
		this->samplePoints = samplePoints;
		this->widthDivision = widthDivision;
		this->heightDivision = heightDivision;
	}

	// vec[y][x] where y goes from top to bottom and x goes from left to right
	std::vector<std::vector<juce::Colour>> GetGridColours() const {
		return snapshotOutput;
	}

	// the colours are ordered from left up to right down
	std::vector<juce::Colour> GetIndexBasedColours() {
		auto colours = GetGridColours();
		std::vector<juce::Colour> indexBasedColours;
		indexBasedColours.reserve(widthDivision * heightDivision);

		for (const auto& xVec : colours) {
			for (auto colour : xVec) {
				indexBasedColours.push_back(colour);
			}
		}

		return indexBasedColours;
	}
};


}
