/*
  ==============================================================================

    CustomLookAndFeel.cpp
    Created: 16 Feb 2022 3:53:20pm
    Author:  Michael

  ==============================================================================
*/

#include "CustomLookAndFeel.h"

void customButton::drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
    bool isHighlighted, bool isButtonDown)
{
    juce::Image background;
    if (isButtonDown)
        background = juce::ImageCache::getFromMemory(BinaryData::cogdown_png, BinaryData::cogdown_pngSize);
    else if (isHighlighted)
        background = juce::ImageCache::getFromMemory(BinaryData::coghighlight_png, BinaryData::coghighlight_pngSize);
    else
        background = juce::ImageCache::getFromMemory(BinaryData::cogwheel2_png, BinaryData::cogwheel2_pngSize);
    g.drawImageWithin(background, 0, 0, button.getWidth(), button.getHeight(), juce::RectanglePlacement());
}
