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

void PresetManager::savePreset(juce::File newInstrument)
{
    // check if the file passed in actually exists.
    // if it does, delete it. if not, create it

    if (newInstrument.exists())
    {
        newInstrument.deleteFile();
    }
    else
    {
        newInstrument.create();
    }

    // update preset name for GUI display using file name
    currentPresetName = newInstrument.getFileNameWithoutExtension();

    // allocate memory on the stack using MemoryBlock, and fill it with our preset data
    // using get state information

    // write the preset data to the file

    // update the preset list
}

void PresetManager::saveChooser()
{
    chooser = std::make_unique<juce::FileChooser>("Save a microtonal mapping preset", juce::File::getSpecialLocation(juce::File::userDocumentsDirectory), "*xml", true, false);
    auto flags = juce::FileBrowserComponent::saveMode
        | juce::FileBrowserComponent::canSelectFiles
        | juce::FileBrowserComponent::warnAboutOverwriting;
    //juce::String mapping = microtonalMappings[preset].generateValueTree().toXmlString();
    chooser->launchAsync(flags, [this](const juce::FileChooser& fc) {
        if (fc.getResult() == juce::File{})
            return;
        juce::File myFile = fc.getResult().withFileExtension("xml");
        juce::String fileName = myFile.getFileName();
        /* Save file logic goes here*/
        /*if (!myFile.replaceWithText(mapping)) {
            juce::AlertWindow::showMessageBoxAsync(
                juce::AlertWindow::WarningIcon,
                TRANS("Error whilst saving"),
                TRANS("Couldn't write to the specified file!")
            );
        }*/
        /* End save file logic*/
        });
}

void PresetManager::loadPreset(juce::File presetToLoad)
{
    // create a MemoryBlock onto which we load the preset data

    // if file load is successful, update current preset name and call setStateInformation
    // with loaded data
}

/*
    const juce::String PresetManager::getPresetName(int inPresetIndex)
    {
        // return name of preset at given index
    }

    const juce::String PresetManager::getCurrentPresetName()
    {
        // return name of currently loaded preset
    }

    const int PresetManager::getNumberOfPresets()
    {
        // return size of preset list
    }

    const juce::String PresetManager::getCurrentPresetDirectory()
    {
        // return path of root preset directory as a juce::String
    }


*/

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
