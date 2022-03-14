/*
  ==============================================================================

    PresetListBox.h
    Created: 22 Jan 2022 10:01:32pm
    Author:  Michael

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
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

    void changeListenerCallback(juce::ChangeBroadcaster*) override
    {
        presets = settings->settings.getOrCreateChildWithName("presets", nullptr);
        // forward to ListBox
        sendChangeMessage();
    }

    std::function<void(int rowNumber)> onSelectionChanged;

    void updateInstrumentList()
    {
        instrumentList.clear();

        // Iterate through the directory and get each file
        auto iterator = juce::RangedDirectoryIterator(juce::File(presetDirectory), false, presetWildCard, juce::File::TypesOfFileToFind::findFiles);

        for (auto f : iterator)
        {
            instrumentList.add(f.getFile());
        }
    }

private:
    juce::ValueTree presets;
    foleys::SharedApplicationSettings settings;

    std::unique_ptr<juce::FileChooser> chooser;

    juce::File presetDirectory;

    juce::String currentPresetName{ "Untitled" };

    juce::Array<juce::File> instrumentList;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetListBox)
};
