/*
  ==============================================================================

    CustomLookAndFeel.cpp
    Created: 16 Feb 2022 3:53:20pm
    Author:  Michael

  ==============================================================================
*/

#include "CustomLookAndFeel.h"

void customSettings::drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
    bool isHighlighted, bool isButtonDown)
{
    button.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    juce::Image background;
    if (isButtonDown)
        background = juce::ImageCache::getFromMemory(BinaryData::cogdown_png, BinaryData::cogdown_pngSize);
    else if (isHighlighted)
        background = juce::ImageCache::getFromMemory(BinaryData::coghighlight_png, BinaryData::coghighlight_pngSize);
    else
        background = juce::ImageCache::getFromMemory(BinaryData::cogwheel2_png, BinaryData::cogwheel2_pngSize);
    g.drawImageWithin(background, 0, 0, button.getWidth(), button.getHeight(), juce::RectanglePlacement());
}

void customSave::drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
    bool isHighlighted, bool isButtonDown)
{
    button.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    juce::Image background;
    if (isButtonDown)
        background = juce::ImageCache::getFromMemory(BinaryData::savedown_png, BinaryData::savedown_pngSize);
    else if (isHighlighted)
        background = juce::ImageCache::getFromMemory(BinaryData::saveover_png, BinaryData::saveover_pngSize);
    else
        background = juce::ImageCache::getFromMemory(BinaryData::saveFile_png, BinaryData::saveFile_pngSize);
    g.drawImageWithin(background, 0, 0, button.getWidth(), button.getHeight(), juce::RectanglePlacement());
}

void customLoad::drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
    bool isHighlighted, bool isButtonDown)
{
    button.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    juce::Image background;
    if (isButtonDown)
        background = juce::ImageCache::getFromMemory(BinaryData::downloaddown_png, BinaryData::downloaddown_pngSize);
    else if (isHighlighted)
        background = juce::ImageCache::getFromMemory(BinaryData::downloadover_png, BinaryData::downloadover_pngSize);
    else
        background = juce::ImageCache::getFromMemory(BinaryData::download_png, BinaryData::download_pngSize);
    g.drawImageWithin(background, 0, 0, button.getWidth(), button.getHeight(), juce::RectanglePlacement());
}

void customPower::drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
    bool isHighlighted, bool isButtonDown)
{
    button.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    juce::Image background;
    background = juce::ImageCache::getFromMemory(BinaryData::power_png, BinaryData::power_pngSize);
    g.drawImageWithin(background, 0, 0, button.getWidth(), button.getHeight(), juce::RectanglePlacement());
}

