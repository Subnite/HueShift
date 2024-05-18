#pragma once
#include "juce_core/juce_core.h"
#include "juce_graphics/juce_graphics.h"
#include <vector>

#ifndef C1
#define C1 24
#endif

namespace HueShift{

inline int NoteToGridIndex(int noteNumber) {
	return noteNumber - C1;
}

class ColorInfo {
public:
	float frequency;
	juce::Range<float> hue;
	ColorInfo(float frequency, juce::Range<float> hue)
	: frequency(frequency), hue(hue)
	{}

	static ColorInfo GetClosestColor(juce::Colour c){
		auto hue = c.getHue(); // [0:1]

		auto colors = GetColors();
		for (auto& color : colors) {
			if (color.hue.contains(hue)) return color;
		}

		// if not found then it's higher than pink aka red
		return red;
	}

	static std::vector<ColorInfo> GetColors() {
		return std::vector<ColorInfo>({red, orange, yellow, green, cyan, blue, violet, pink});
	}

	static const ColorInfo red;
	static const ColorInfo orange;
	static const ColorInfo yellow;
	static const ColorInfo green;
	static const ColorInfo cyan;
	static const ColorInfo blue;
	static const ColorInfo violet;
	static const ColorInfo pink;
};

inline const ColorInfo ColorInfo::red 		= 	ColorInfo(4.5f, 			juce::Range<float>(0.000f, 0.042f));
inline const ColorInfo ColorInfo::orange 	= 	ColorInfo(5.f, 				juce::Range<float>(0.042f, 0.111f));
inline const ColorInfo ColorInfo::yellow 	= 	ColorInfo(5.166666666f, 	juce::Range<float>(0.111f, 0.172f));
inline const ColorInfo ColorInfo::green 	= 	ColorInfo(5.5f, 			juce::Range<float>(0.172f, 0.431f));
inline const ColorInfo ColorInfo::cyan 		= 	ColorInfo(6.0f, 			juce::Range<float>(0.431f, 0.494f));
inline const ColorInfo ColorInfo::blue 		= 	ColorInfo(6.666666666f, 	juce::Range<float>(0.494f, 0.694f));
inline const ColorInfo ColorInfo::violet 	= 	ColorInfo(7.5f, 			juce::Range<float>(0.694f, 0.806f));
inline const ColorInfo ColorInfo::pink 		= 	ColorInfo(8.333333333f, 	juce::Range<float>(0.806f, 0.944f));
}