/*
  ==============================================================================

    PresetListBox.h
    Created: 22 Jan 2022 10:01:32pm
    Author:  Michael

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

static const juce::String presetFileExt = ".xml";
static const juce::String presetWildCard = "*.xml";


#if JUCE_WINDOWS
static const juce::String directorySeparator = "\\";

#elif JUCE_MAC
static const juce::String directorySeparator = "//";

#endif

class PresetListBox : public juce::ListBoxModel,
    public juce::ChangeBroadcaster,
    public juce::ChangeListener
{
public:
    PresetListBox()
    {
        settings->addChangeListener(this);
    }

    ~PresetListBox() override
    {
        settings->removeChangeListener(this);
    }

    void setPresetsNode(juce::ValueTree node)
    {
        presets = node;
        sendChangeMessage();
    }

    int getNumRows() override
    {
        return presets.getNumChildren();
    }

    void listBoxItemClicked(int rowNumber, const juce::MouseEvent& event) override
    {
        if (event.mods.isPopupMenu())
        {
            juce::PopupMenu::Options options;
            juce::PopupMenu menu;
            menu.addItem("Remove", [this, rowNumber]()
            {
                presets.removeChild(rowNumber, nullptr);
            });
            menu.showMenuAsync(options);
        }

        if (onSelectionChanged)
            onSelectionChanged(rowNumber);
    }

    void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override
    {
        auto bounds = juce::Rectangle<int>(0, 0, width, height);
        if (rowIsSelected)
        {
            g.setColour(juce::Colours::grey);
            g.fillRect(bounds);
        }

        g.setColour(juce::Colours::silver);
        g.drawFittedText(presets.getChild(rowNumber).getProperty("name", "foo").toString(), bounds, juce::Justification::centredLeft, 1);
    }

    void updateInstrumentList()
    {
        juce::File path = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory);
        
        if (path.isDirectory())
            DBG("Success, is a directory");
        /*instrumentList.clear();*/

        // Iterate through the directory and get each file
        //auto iterator = juce::RangedDirectoryIterator(juce::File(path), false, presetWildCard, juce::File::TypesOfFileToFind::findFiles);

        for (juce::DirectoryEntry f : juce::RangedDirectoryIterator(juce::File(path), false, presetWildCard))
        {
            juce::File fileToRead = f.getFile();
            juce::ValueTree convertedFile = juce::ValueTree::fromXml(*juce::XmlDocument(fileToRead).getDocumentElement());

            DBG("Inside of updateInstrumentList");
            DBG(convertedFile.toXmlString());

            presets.addChild(convertedFile, -1, nullptr);
            //instrumentList.add(f.getFile());
            //presets.addChild(f.getFile());
        }
    }

    juce::ValueTree getInstrument(int index)
    {
        juce::ValueTree vt = presets.getChild(index);
        return vt;
    }

    void changeListenerCallback(juce::ChangeBroadcaster*) override
    {
        presets = settings->settings.getOrCreateChildWithName("presets", nullptr);
        
        // forward to ListBox
        sendChangeMessage();
    }

    std::function<void(int rowNumber)> onSelectionChanged;

private:
    juce::ValueTree presets;
    foleys::SharedApplicationSettings settings;

    std::unique_ptr<juce::FileChooser> chooser;


    juce::String currentPresetName{ "Untitled" };

    juce::Array<juce::File> instrumentList;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetListBox)
};
