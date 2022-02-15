/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/
#pragma once

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "PresetListBox.h"
#include <string> 
#include <cctype> 
#include <math.h>
#include <iostream>
#include <algorithm>
using namespace std;

//==============================================================================
MicrotonalWindow::MicrotonalWindow(juce::String name) : DocumentWindow(name,
    juce::Colours::dimgrey,
    DocumentWindow::closeButton)
{
    double ratio = 2; // adjust as desired
    setContentOwned(new MainContentComponent(), true);
    //centreWithSize(1400, 700/ratio);
    getConstrainer()->setFixedAspectRatio(ratio);
    centreWithSize(1400, 700);
    setResizable(true, true);
    setResizeLimits(800, 600/ratio, 1800, 1600/ratio);
    setVisible(true);
}

void MicrotonalWindow::closeButtonPressed()
{
    // juce::JUCEApplication::getInstance()->systemRequestedQuit();
    delete this;
}

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    Synth::addADSRParameters(layout);
    Synth::addOvertoneParameters(layout);
    Synth::addGainParameters(layout);
    return layout;
}
MicrotonalSynthAudioProcessorEditor::MicrotonalSynthAudioProcessorEditor()
    : foleys::MagicProcessor(juce::AudioProcessor::BusesProperties()
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
    treeState(*this, nullptr, ProjectInfo::projectName, createParameterLayout())
{
    FOLEYS_SET_SOURCE_PATH(__FILE__);

    auto file = juce::File::getSpecialLocation(juce::File::currentApplicationFile)
        .getChildFile("Contents")
        .getChildFile("Resources")
        .getChildFile("layout.xml");

    if (file.existsAsFile())
        magicState.setGuiValueTree(file);
    else
        magicState.setGuiValueTree(BinaryData::mappinglayoutv1_xml, BinaryData::mappinglayoutv1_xmlSize);

    // MAGIC GUI: add a meter at the output
    outputMeter = magicState.createAndAddObject<foleys::MagicLevelSource>("output");
    oscilloscope = magicState.createAndAddObject<foleys::MagicOscilloscope>("waveform");

    analyser = magicState.createAndAddObject<foleys::MagicAnalyser>("analyser");
    magicState.addBackgroundProcessing(analyser);

    presetList = magicState.createAndAddObject<PresetListBox>("presets");
    presetList->onSelectionChanged = [&](int number)
    {
        loadPresetInternal(number);
    };
    magicState.addTrigger("save-preset", [this]
    {
        savePresetInternal();
    });
    magicState.addTrigger("open-window", [this]
    {
        openWindow();
    });

    magicState.setApplicationSettingsFile(juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile(ProjectInfo::companyName)
        .getChildFile(ProjectInfo::projectName + juce::String(".settings")));

    magicState.setPlayheadUpdateFrequency(30);

    Synth::Sound::Ptr sound(new Synth::Sound(treeState));
    synthesiser.addSound(sound);

    for (int i = 0; i < 16; ++i)
        synthesiser.addVoice(new Synth::Voice(treeState));
}

MicrotonalSynthAudioProcessorEditor::~MicrotonalSynthAudioProcessorEditor()
{
    if (window)
        delete window;
}
void MicrotonalSynthAudioProcessorEditor::prepareToPlay(double sampleRate, int blockSize)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    synthesiser.setCurrentPlaybackSampleRate(sampleRate);

    // MAGIC GUI: setup the output meter
    outputMeter->setupSource(getTotalNumOutputChannels(), sampleRate, 500);
    oscilloscope->prepareToPlay(sampleRate, blockSize);
    analyser->prepareToPlay(sampleRate, blockSize);
}

void MicrotonalSynthAudioProcessorEditor::openWindow()
{
    if(!window)
        window = new MicrotonalWindow("Configure Microtonal Mapping");
}

void MicrotonalSynthAudioProcessorEditor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool MicrotonalSynthAudioProcessorEditor::isBusesLayoutSupported(const juce::AudioProcessor::BusesLayout& layouts) const
{
    // This synth only supports mono or stereo.
    return (layouts.getMainOutputChannelSet() == juce::AudioChannelSet::mono()
        || layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo());
}

void MicrotonalSynthAudioProcessorEditor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    // MAGIC GUI: send midi messages to the keyboard state and MidiLearn
    magicState.processMidiBuffer(midiMessages, buffer.getNumSamples(), true);

    // MAGIC GUI: send playhead information to the GUI
    magicState.updatePlayheadInformation(getPlayHead());

    synthesiser.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
	
    for (int i = 1; i < buffer.getNumChannels(); ++i)
        buffer.copyFrom(i, 0, buffer.getReadPointer(0), buffer.getNumSamples());

    // MAGIC GUI: send the finished buffer to the level meter
    outputMeter->pushSamples(buffer);
    oscilloscope->pushSamples(buffer);
    analyser->pushSamples(buffer);
}

//==============================================================================

//==============================================================================
void MicrotonalSynthAudioProcessorEditor::savePresetInternal()
{
    presetNode = magicState.getSettings().getOrCreateChildWithName("presets", nullptr);

    juce::ValueTree preset{ "Preset" };
    preset.setProperty("name", "Preset " + juce::String(presetNode.getNumChildren() + 1), nullptr);

    foleys::ParameterManager manager(*this);
    manager.saveParameterValues(preset);

    presetNode.appendChild(preset, nullptr);
}

void MicrotonalSynthAudioProcessorEditor::loadPresetInternal(int index)
{
    presetNode = magicState.getSettings().getOrCreateChildWithName("presets", nullptr);
    auto preset = presetNode.getChild(index);

    foleys::ParameterManager manager(*this);
    manager.loadParameterValues(preset);
}

//==============================================================================

double MicrotonalSynthAudioProcessorEditor::getTailLengthSeconds() const
{
    return 0.0;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MicrotonalSynthAudioProcessorEditor();
}



//class MainContentComponentItem : public foleys::GuiItem
//{
//public:
//    FOLEYS_DECLARE_GUI_FACTORY(MainContentComponentItem)
//
//        MainContentComponentItem(foleys::MagicGUIBuilder& builder, const juce::ValueTree& node) : foleys::GuiItem(builder, node)
//    {
//
//        addAndMakeVisible(maincontentcomponent);
//    }
//
//    std::vector<foleys::SettableProperty> getSettableProperties() const override
//    {
//        std::vector<foleys::SettableProperty> newProperties;
//
//        newProperties.push_back({ configNode, "factor", foleys::SettableProperty::Number, 1.0f, {} });
//
//        return newProperties;
//    }
//
//    // Override update() to set the values to your custom component
//    void update() override
//    {
//        auto factor = getProperty("factor");
//    }
//
//    juce::Component* getWrappedComponent() override
//    {
//        return &maincontentcomponent;
//    }
//
//private:
//    MainContentComponent maincontentcomponent;
//
//    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainContentComponentItem)
//};



//juce::AudioProcessorEditor* MicrotonalSynthAudioProcessorEditor::createEditor()
//{
//    // MAGIC GUI: we create our custom builder instance here, that will be available for all factories we add
//    auto builder = std::make_unique<foleys::MagicGUIBuilder>(magicState);
//    builder->registerJUCEFactories();
//
//    builder->registerFactory("MainContentComponentItem", &MainContentComponentItem::factory);
//
//    return new foleys::MagicPluginEditor(magicState, std::move(builder));
//}

