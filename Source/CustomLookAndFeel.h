/*
  ==============================================================================

    CustomLookAndFeel.h
    Created: 16 Feb 2022 3:53:20pm
    Author:  Michael

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
class customButton : public juce::LookAndFeel_V4
{
public:
    customButton() {};
    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
        bool isHighlighted, bool isButtonDown) override;
};