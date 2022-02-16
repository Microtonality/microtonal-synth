/*
  ==============================================================================

    CustomLookAndFeel.cpp
    Created: 16 Feb 2022 3:53:20pm
    Author:  Michael

  ==============================================================================
*/

#include "CustomLookAndFeel.h"

void customButton::drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
    bool, bool isButtonDown)
{
    juce::Image background = juce::ImageCache::getFromMemory(BinaryData::cogwheelMedium_png, BinaryData::cogwheelMedium_pngSize);
    //g.drawImageAt(background, 0, 0);
    g.drawImageWithin(background, 0, 0, button.getWidth(), button.getHeight(), juce::RectanglePlacement());
}
