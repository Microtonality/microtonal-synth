/*
  ==============================================================================

    PresetManager.h
    Created: 9 Feb 2022 1:51:09pm
    Author:  John

  ==============================================================================
*/


#pragma once

#include "JuceHeader.h"
class PresetManager {


public:
    PresetManager(juce::AudioProcessor* inProcessor);

    void createNewPreset();

    void savePreset();

    void loadPreset();

    const juce::String getPresetName();


private:


    void updatePresetList();

    juce::File presetDirectory;

    juce::String currentInstrument{ "Untitled" };


};