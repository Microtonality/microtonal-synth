/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class MicrotonalSynthAudioProcessor  : public foleys::MagicProcessor
{
public:
    
    //==============================================================================
    MicrotonalSynthAudioProcessor();
    ~MicrotonalSynthAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;
    
    //==============================================================================
    juce::AudioProcessorValueTreeState treeState{ *this, nullptr };
   // juce::AudioProcessorValueTreeState treeState;
    //foleys::MagicProcessorState magicState;
    //juce::AudioProcessorEditor* MicrotonalSynthAudioProcessor::createEditor() override;
    void MicrotonalSynthAudioProcessor::initialiseBuilder(foleys::MagicGUIBuilder& builder);

private:
    //==============================================================================
    int divisions;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MicrotonalSynthAudioProcessor)
};
