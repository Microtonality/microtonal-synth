/*
  ==============================================================================

    PresetManager.cpp
    Created: 24 Feb 2022 4:58:15pm
    Author:  John

  ==============================================================================
*/

#include "PresetManager.h"

PresetManager::PresetManager(juce::AudioProcessor* inProcessor)
{
    presetDirectory = juce::File((juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)).getFullPathName());

    if (!presetDirectory.exists())
        presetDirectory.createDirectory();

    updatePresetList();
}





void PresetManager::createNewPreset()
{
    // here you would iterate through all of the APVTS parameters and set them
    // to their default values, then set currentPresetName to "Untitled" or some init. state
}

void PresetManager::savePreset(juce::File presetToSave)
{
    // check if the file passed in actually exists.
    // if it does, delete it. if not, create it

    // update preset name for GUI display using file name

    // allocate memory on the stack using MemoryBlock, and fill it with our preset data
    // using get state information

    // write the preset data to the file

    // update the preset list
}

void PresetManager::loadPreset(juce::File presetToLoad)
{
    // create a MemoryBlock onto which we load the preset data

    // if file load is successful, update current preset name and call setStateInformation
    // with loaded data
}


const juce::String PresetManager::getPresetName(int inPresetIndex)
{  
    // return name of preset at given index
    juce::String presetName = "";
    return presetName;
}

const juce::String PresetManager::getCurrentPresetName()
{
    // return name of currently loaded preset
    juce::String presetName = "";
    return presetName;
}

const int PresetManager::getNumberOfPresets()
{
    // return size of preset list
    return 0;
}

const juce::String PresetManager::getCurrentPresetDirectory()
{
    // return path of root preset directory as a juce::String
    juce::String presetDir = "";
    return presetDir;
}

void PresetManager::setCurrentPresetName(juce::String newPresetName)
{
    // set the current preset name
}

void PresetManager::updatePresetList()
{
    presetList.clear();

    // Iterate through the directory and get each file
    auto iterator = juce::RangedDirectoryIterator(juce::File(presetDirectory), false, presetWildCard, juce::File::TypesOfFileToFind::findFiles);
}
