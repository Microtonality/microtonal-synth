  /*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
//#include "PluginProcessor.h"
#include "../components/microtonal/MicrotonalMapper.h"
#include "CustomLookAndFeel.h"
#include "../audioProcessor/synth.h"
#include <atomic> 

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
    MicrotonalWindow(juce::String name, int index);
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
    void deletePreset(int toDelete);
    void loadAllInstruments();

    //==============================================================================
    double getTailLengthSeconds() const override;

    void initialiseBuilder(foleys::MagicGUIBuilder& builder) override;

    void loadMicrotonalPreset(int);

    void saveMicrotonalPreset(int);

    void openWindow(int index);


private:
    juce::AudioProcessorValueTreeState treeState;
    juce::Component::SafePointer<MicrotonalWindow> window;
    int activeWindow = Default;
    Synth      synthesiser;
    juce::ValueTree  presetNode, microtonalNode;
    std::unique_ptr<juce::FileChooser> chooser;
    // GUI MAGIC: define that as last member of your AudioProcessor
    foleys::MagicLevelSource* outputMeter = nullptr;
    foleys::MagicPlotSource* oscilloscope = nullptr;
    foleys::MagicPlotSource* analyser = nullptr;

    PresetListBox* presetList = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MicrotonalSynthAudioProcessorEditor)
};

