/*
  ==============================================================================

    CustomLookAndFeel.h
    Created: 16 Feb 2022 3:53:20pm
    Author:  Michael

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
class customSettings : public juce::LookAndFeel_V4
{
public:
    customSettings() {};
    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
        bool isHighlighted, bool isButtonDown) override;
};

class customSave : public juce::LookAndFeel_V4
{
public:
    customSave() {};
    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
        bool isHighlighted, bool isButtonDown) override;
};

class customLoad : public juce::LookAndFeel_V4
{
public:
    customLoad() {};
    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
        bool isHighlighted, bool isButtonDown) override;
};

class customPower : public juce::LookAndFeel_V4
{
public:
    customPower() {};
    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
        bool isHighlighted, bool isButtonDown) override;
};