/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/
#pragma once

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "PresetListBox.h"
#include "CustomLookAndFeel.h"
#include <string> 
#include <cctype> 
#include <math.h>
#include <iostream>
#include <algorithm>
using namespace std;
int mappingGroup = Default;
extern MicrotonalConfig microtonalMappings[7];
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
        magicState.setGuiValueTree(BinaryData::layout_xml, BinaryData::layout_xmlSize);

    // MAGIC GUI: add a meter at the output
    outputMeter = magicState.createAndAddObject<foleys::MagicLevelSource>("output");
    oscilloscope = magicState.createAndAddObject<foleys::MagicOscilloscope>("waveform");
    analyser = magicState.createAndAddObject<foleys::MagicAnalyser>("analyser");
    magicState.addBackgroundProcessing(analyser);

    presetList = magicState.createAndAddObject<PresetListBox>("presets");
    savePresetInternal();
    loadPresetInternal(0);
  //  presetNode.getChildWithName("presets");
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
    magicState.addTrigger("set-map1", [this]
    {
        mappingGroup = mappingGroup == Group1 ? Default : Group1;
        DBG(mappingGroup);
        
    });
    magicState.addTrigger("set-map2", [this]
    {
        mappingGroup = mappingGroup == Group2 ? Default : Group2;
        DBG(mappingGroup);
    });
    magicState.addTrigger("set-map3", [this]
    {
        mappingGroup = mappingGroup == Group3 ? Default : Group3;
        DBG(mappingGroup);
    });
    magicState.addTrigger("set-map4", [this]
    {
        mappingGroup = mappingGroup == Group4 ? Default : Group4;
        DBG(mappingGroup);
    });
    magicState.addTrigger("set-map5", [this]
    {
        mappingGroup = mappingGroup == Group5 ? Default : Group5;
        DBG(mappingGroup);
    });
    magicState.addTrigger("set-map6", [this]
    {
        mappingGroup = mappingGroup == Group6 ? Default : Group6;
        DBG(mappingGroup);
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
    //magicState.getSettings().getOrCreateChildWithName("presets", nullptr) = nullptr;
    magicState.getSettings().removeAllChildren(nullptr);
    presetNode = magicState.getSettings().getOrCreateChildWithName("presets", nullptr);

    juce::ValueTree preset{ "Preset" };
   // preset.setProperty("name", "Preset " + juce::String(presetNode.getNumChildren() + 1), nullptr);
    preset.setProperty("name", "Preset", nullptr);
    foleys::ParameterManager manager(*this);
    manager.saveParameterValues(preset);

    presetNode.appendChild(preset, nullptr);
    DBG(presetNode.toXmlString());
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
class ActivePresetComponent : public juce::Component, private juce::Timer
{
public:
    //==============================================================================
    ActivePresetComponent(){
        for (int i = 0; i < 7; i++) {
            addAndMakeVisible(btns[i]);
            btns[i].setButtonText(to_string(i));
            btns[i].setEnabled(false);
        }
        startTimerHz(30);
    };
    void setFactor(float f)
    {
        factor = f;
    }
    void resized() override{
        auto rect = getLocalBounds();
        rect.setWidth(rect.getWidth() / 7);
        for (int i = 0; i < 7; i++) {
            btns[i].setBounds(rect);
            rect.setX(rect.getX() + rect.getWidth());
        }
    }
    void paint(juce::Graphics& g) override{
        for (int i = 0; i < 7; i++) {
            if (i == mappingGroup) {
                btns[i].setColour(juce::TextButton::buttonColourId, juce::Colours::darkgreen);
            } else {
                btns[i].setColour(juce::TextButton::buttonColourId, juce::Colours::red);
            }
        }
    }
private:
    juce::TextButton btns[7];
    void timerCallback() override
    {
        phase += 0.1f;
        if (phase >= juce::MathConstants<float>::twoPi)
            phase -= juce::MathConstants<float>::twoPi;

        repaint();
    }

    float factor = 3.0f;
    float phase = 0.0f;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ActivePresetComponent)
};
    
class ActivePresetComponentItem : public foleys::GuiItem
{
public:
    FOLEYS_DECLARE_GUI_FACTORY(ActivePresetComponentItem)

    ActivePresetComponentItem(foleys::MagicGUIBuilder& builder, const juce::ValueTree& node) : foleys::GuiItem(builder, node)
    {
        addAndMakeVisible(activepresetcomponent);
    }

    void update() override
    {
       // activepresetcomponent.repaint();
        auto factor = getProperty("factor");
        activepresetcomponent.setFactor(factor.isVoid() ? 3.0f : float(factor));
    }

    juce::Component* getWrappedComponent() override
    {
        return &activepresetcomponent;
    }

private:
    ActivePresetComponent activepresetcomponent;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ActivePresetComponentItem)
};
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

void MicrotonalSynthAudioProcessorEditor::initialiseBuilder(foleys::MagicGUIBuilder& builder)
{
    builder.registerJUCEFactories();
    builder.registerJUCELookAndFeels();
    builder.registerLookAndFeel("Settings", make_unique<customButton>());
    builder.registerFactory("ActivePresetComponent", &ActivePresetComponentItem::factory);
}

//juce::AudioProcessorEditor* MicrotonalSynthAudioProcessorEditor::createEditor()
//{
//    // MAGIC GUI: we create our custom builder instance here, that will be available for all factories we add
//    auto builder = std::make_unique<foleys::MagicGUIBuilder>(magicState);
//    builder->registerJUCEFactories();
//    builder->registerLookAndFeel("Settings", make_unique<customButton>());
//
//    return new foleys::MagicPluginEditor(magicState, std::move(builder));
//}

