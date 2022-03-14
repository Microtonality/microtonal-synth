/*
  ==============================================================================

    PresetManager.h
    Created: 24 Feb 2022 4:58:15pm
    Author:  John

  ==============================================================================
*/

#pragma once

#include "JuceHeader.h"
#include "PresetListBox.h"

static const juce::String presetFileExt = ".xml";
static const juce::String presetWildCard = "*.xml";


#if JUCE_WINDOWS
static const juce::String directorySeparator = "\\";

#elif JUCE_MAC
static const juce::String directorySeparator = "//";

#endif

// define a preset file name extension
// define a preset file name wildcard

// define OS dependent directory separators


class PresetManager
{
public:

    PresetManager(juce::AudioProcessor* inProcessor);

    /** Sets all parameters to default and sets preset name to "Untitled" */
    void createNewPreset();

    /**  Takes a user selected file and saves the APVTS state to it */
    void savePreset(juce::File presetToSave);

    /**  Takes a user selected file and loads data from it into the APVTS */
    void loadPreset(juce::File presetToLoad);

  
    void saveChooser();

    /** Returns the present name for a given index of the preset directory */
    const juce::String getPresetName(int inPresetIndex);

    /** Returns the name of the currently loaded preset */
    const juce::String getCurrentPresetName();

    /** Returns the base Preset Directory */
    const juce::String getCurrentPresetDirectory();

    /** Allows caller to set the name of the currently loaded preset. used to facilitate state restore */
    void setCurrentPresetName(juce::String newPresetName);

    /** Returns the number of presets loaded to the local presets array */
    const int getNumberOfPresets();

    void loadAllInstruments();


private:

    /** Iterates over the preset directory and adds the files to localPresets  */
    void updatePresetList();

    std::unique_ptr<juce::FileChooser> chooser;

    juce::File presetDirectory;
    
    juce::String currentPresetName{ "Untitled" };
    
    juce::Array<juce::File> presetList;
    
    juce::AudioProcessor* processor;
};
