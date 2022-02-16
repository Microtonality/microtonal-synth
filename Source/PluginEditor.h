  /*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
//#include "PluginProcessor.h"
#include "MicrotonalMapper.h"
#include "CustomLookAndFeel.h"
#include "synth.h"

class PresetListBox;
//==============================================================================
/**
*/
enum SelectMappingGroup {
    Group1 = 1,
    Group2 = 2,
    Group3 = 3,
    Group4 = 4,
    Group5 = 5,
    Group6 = 6,
    Default = 0
};
extern int mappingGroup;
class MicrotonalWindow : public juce::DocumentWindow
{
public:
    MicrotonalWindow(juce::String name);
    void closeButtonPressed() override;

    //   void resized() override;

private:

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MicrotonalWindow)

};

class MicrotonalSynthAudioProcessorEditor  : public foleys::MagicProcessor
{
public:
    //==============================================================================
    MicrotonalSynthAudioProcessorEditor();
    ~MicrotonalSynthAudioProcessorEditor();
    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const juce::AudioProcessor::BusesLayout& layouts) const override;
    #endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================

    //==============================================================================
    void savePresetInternal();
    void loadPresetInternal(int index);

    //==============================================================================
    double getTailLengthSeconds() const override;

    void initialiseBuilder(foleys::MagicGUIBuilder& builder) override;

    void openWindow();


private:
    juce::AudioProcessorValueTreeState treeState;
    juce::Component::SafePointer<MicrotonalWindow> window;
    Synth      synthesiser;
    juce::ValueTree  presetNode;

    // GUI MAGIC: define that as last member of your AudioProcessor
    foleys::MagicLevelSource* outputMeter = nullptr;
    foleys::MagicPlotSource* oscilloscope = nullptr;
    foleys::MagicPlotSource* analyser = nullptr;
    customButton settings;

    PresetListBox* presetList = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MicrotonalSynthAudioProcessorEditor)
};

