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
juce::String microtonalPresetNames[7] = { "Default", "1", "2", "3", "4", "5", "6" };


//==============================================================================
MicrotonalWindow::MicrotonalWindow(juce::String name, int index) : DocumentWindow(name,
    juce::Colours::dimgrey,
    DocumentWindow::closeButton | DocumentWindow::maximiseButton, true)
{
    double ratio = 2; // adjust as desired
    setContentOwned(new MainContentComponent(index), true);
    //centreWithSize(1400, 700/ratio);
    getConstrainer()->setFixedAspectRatio(ratio);
    centreWithSize(1300, 600);
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

    auto groupInstruments = std::make_unique<juce::AudioProcessorParameterGroup>("instruments", "Instruments", "|");
    groupInstruments->addChild(std::make_unique<juce::AudioParameterChoice>("instrumentPreset", "Instrument_Preset", juce::StringArray({ "preset342", "preset54" }), 0));
    layout.add(std::move(groupInstruments));

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

    //loadAllInstruments();

    //presetNode = magicState.getSettings().getOrCreateChildWithName("presets", nullptr);

    //magicState.createAndAddObject<vector<juce::String>>("instruments", instrumentNames);

    presetList = magicState.createAndAddObject<PresetListBox>("presets");
    //savePresetInternal();

    //magicState.getSettings().getChildWithName("instrument_presets").removeAllChildren(nullptr);
    //DBG(magicState.getSettings().toXmlString());
    //loadPresetInternal(0);
    
  //  presetNode.getChildWithName("presets");
    presetList->onSelectionChanged = [&](int number)
    {
        loadPresetInternal(number);
    };
    magicState.addTrigger("save-preset", [this]
    {
        savePresetInternal();
    });
    magicState.addTrigger("load-microtonal-preset1", [this]
    {
            loadMicrotonalPreset(1);
    });
    magicState.addTrigger("save-microtonal-preset1", [this]
    {
            saveMicrotonalPreset(1);
    });
    magicState.addTrigger("load-microtonal-preset2", [this]
    {
            loadMicrotonalPreset(2);
    });
    magicState.addTrigger("save-microtonal-preset2", [this]
    {
            saveMicrotonalPreset(2);
    });
    magicState.addTrigger("load-microtonal-preset3", [this]
    {
            loadMicrotonalPreset(3);
    });
    magicState.addTrigger("save-microtonal-preset3", [this]
    {
            saveMicrotonalPreset(3);
    });
    magicState.addTrigger("load-microtonal-preset4", [this]
    {
            loadMicrotonalPreset(4);
    });
    magicState.addTrigger("save-microtonal-preset4", [this]
    {
            saveMicrotonalPreset(4);
    });
    magicState.addTrigger("load-microtonal-preset5", [this]
    {
            loadMicrotonalPreset(5);
    });
    magicState.addTrigger("save-microtonal-preset5", [this]
    {
            saveMicrotonalPreset(5);
    });
    magicState.addTrigger("load-microtonal-preset6", [this]
    {
            loadMicrotonalPreset(6);
    });
    magicState.addTrigger("save-microtonal-preset6", [this]
    {
            saveMicrotonalPreset(6);
    });
    magicState.addTrigger("open-window1", [this]
    {
       if (activeWindow != 1)
            delete window;
        openWindow(1);
    });
    magicState.addTrigger("open-window2", [this]
    {
       if (activeWindow != 2)
            delete window;
        openWindow(2);
    });
    magicState.addTrigger("open-window3", [this]
    {
       if (activeWindow != 3)
            delete window;
        openWindow(3);
    });
    magicState.addTrigger("open-window4", [this]
    {
       if (activeWindow != 4)
            delete window;
       openWindow(4);
    });
    magicState.addTrigger("open-window5", [this]
    {
       if (activeWindow != 5)
            delete window;
        openWindow(5);
    });
    magicState.addTrigger("open-window6", [this]
    {
       if (activeWindow != 6)
            delete window;
       openWindow(6);
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

    //updateInstrumentList();

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

void MicrotonalSynthAudioProcessorEditor::openWindow(int index)
{
    if (!window) {
        window = new MicrotonalWindow("Configure Microtonal Mapping Preset " + to_string(index), index);
        activeWindow = index;
    }
    else if (index != activeWindow) {
        delete window;
        new MicrotonalWindow("Configure Microtonal Mapping Preset " + to_string(index), index);
    }
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
    //magicState.getSettings().removeAllChildren(nullptr);
    /*presetNode = magicState.getSettings().getOrCreateChildWithName("presets", nullptr);

    presetNode = magicState.getSettings().getOrCreateChildWithName("instrument_presets", nullptr);*/
    //juce::ValueTree preset{ "Preset" };
    /*microtonalNode = magicState.getSettings().getOrCreateChildWithName("microtonal_presets", nullptr);
    for (int i = 1; i < 7; i++) {
        if (microtonalMappings[i].frequencies[0].frequency == NULL) continue;
        juce::ValueTree microtonal = microtonalMappings[i].generateValueTree();
        microtonal.setProperty("index", juce::String(i), nullptr);
        microtonalNode.appendChild(microtonal, nullptr);
    }
    preset.setProperty("name", "Preset " + juce::String(presetNode.getNumChildren() + 1), nullptr); */
    //preset.setProperty("name", "Preset", nullptr);

    
    //manager.saveParameterValues(preset);
    //presetNode.appendChild(preset, nullptr);

    foleys::ParameterManager manager(*this);

    juce::ValueTree instrument = magicState.getSettings().getOrCreateChildWithName("presets", nullptr);
    manager.saveParameterValues(instrument);
  
    DBG(instrument.toXmlString());
    //magicState.getSettings().getChildWithName("presets").removeAllChildren(nullptr);
    /*magicState.getSettings().removeAllChildren(nullptr);
    magicState.getSettings().getChild(0).toXmlString();*/

    chooser = std::make_unique<juce::FileChooser>("Save an instrument preset", juce::File::getSpecialLocation(juce::File::userDocumentsDirectory), "*xml", true, false);
    auto flags = juce::FileBrowserComponent::saveMode
        | juce::FileBrowserComponent::canSelectFiles
        | juce::FileBrowserComponent::warnAboutOverwriting;
   
    chooser->launchAsync(flags, [this, instrument](const juce::FileChooser& fc) {
        if (fc.getResult() == juce::File{})
            return;
        juce::File myFile = fc.getResult().withFileExtension("xml");
        juce::String fileName = myFile.getFileName();
        /* Save file logic goes here*/
        if (!myFile.replaceWithText(instrument.toXmlString())) {
            juce::AlertWindow::showMessageBoxAsync(
                juce::AlertWindow::WarningIcon,
                TRANS("Error whilst saving"),
                TRANS("Couldn't write to the specified file!")
            );
        }
        
        /* End save file logic*/
     });

    /*manager.saveParameterValues(instrument);

    DBG("Trying to access presetlist");
    presetList->updateInstrumentList();*/

}



void MicrotonalSynthAudioProcessorEditor::loadPresetInternal(int index)
{
    //presetNode = magicState.getSettings().getOrCreateChildWithName("presets", nullptr);
    //auto preset = presetNode.getChild(index);
    auto instrument = presetList->getInstrument(index);
    //microtonalNode = magicState.getSettings().getOrCreateChildWithName("microtonal_presets", nullptr);
    /*DBG(microtonalNode.toXmlString().toStdString());
    for (juce::ValueTree t : microtonalNode) {
        int index = stoi(t.getProperty("index").toString().toStdString());
        microtonalMappings[index].base_frequency = stod(t.getProperty("base_frequency").toString().toStdString());
        microtonalMappings[index].divisions = stod(t.getProperty("total_divisions").toString().toStdString());
        int i = 0;
        for (juce::ValueTree p : t) {
            microtonalMappings[index].frequencies[i].index = stoi(p.getProperty("index").toString().toStdString());
            microtonalMappings[index].frequencies[i].frequency = stod(p.getProperty("value").toString().toStdString());
            i++;
        }
    }*/
    foleys::ParameterManager manager(*this);
    manager.loadParameterValues(instrument);
}



void MicrotonalSynthAudioProcessorEditor::deletePreset(int toDelete)
{
    if (toDelete == 0)
    {
        return;
    }

    juce::ValueTree presetNode = magicState.getSettings().getOrCreateChildWithName("presets", nullptr);
    auto preset = presetNode.getChild(toDelete);
    
    //magicState.getSettings().removeAllChildren(nullptr);
    presetNode = magicState.getSettings().getOrCreateChildWithName("presets", nullptr);

    //juce::ValueTree preset{ "Preset" };
    /*preset.setProperty("name", "Preset " + juce::String(presetNode.getNumChildren() + 1), nullptr);
    preset.setProperty("name", "Preset", nullptr);*/
   
    preset.removeAllChildren(nullptr);
    presetNode.removeChild(toDelete, nullptr);
    foleys::ParameterManager manager(*this);
    manager.saveParameterValues(preset);

    //DBG(presetNode.toXmlString());

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
class ActivePresetComponent : public juce::Component, private juce::Timer, public juce::Button::Listener
{
public:
    //==============================================================================
    ActivePresetComponent(){
        for (int i = 0; i < 6; i++) {
            addAndMakeVisible(btns[i]);
            btns[i].setButtonText(microtonalPresetNames[i+1]);
            //btns[i].setEnabled(false);
            btns[i].addListener(this);
            btns[i].setMouseCursor(juce::MouseCursor::PointingHandCursor);
        }
        startTimerHz(30);
    };
    void setFactor(float f)
    {
        factor = f;
    }
    void resized() override{
        auto rect = getLocalBounds();
        rect.setHeight(rect.getHeight() / 6);
        for (int i = 0; i < 6; i++) {
            btns[i].setBounds(rect);
            rect.setY(rect.getY() + rect.getHeight());
        }
    }
    void paint(juce::Graphics& g) override{
        for (int i = 0; i < 6; i++) {
            if (i+1 == mappingGroup) {
                btns[i].setColour(juce::TextButton::buttonColourId, juce::Colours::darkgreen);
            } 
            else if (microtonalMappings[i + 1].frequencies[0].frequency != NULL) {
                btns[i].setColour(juce::TextButton::buttonColourId, juce::Colours::blue);
            }
            else {
                btns[i].setColour(juce::TextButton::buttonColourId, juce::Colours::grey);
            }
            btns[i].setButtonText(microtonalPresetNames[i + 1].contains(".xml") ? microtonalPresetNames[i + 1].substring(0, microtonalPresetNames[i + 1].indexOf(".")) : microtonalPresetNames[i + 1]);


        }
        

    }
    void buttonClicked(juce::Button* btn) override{
        for (int i = 0; i < 6; i++) {
            if (btn == &btns[i]) {
                mappingGroup = mappingGroup == i + 1 ? Default : i + 1;

            }

        }
    }
private:
    juce::TextButton btns[6];
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
    builder.registerLookAndFeel("Settings", make_unique<customSettings>());
    builder.registerLookAndFeel("Save", make_unique<customSave>());
    builder.registerLookAndFeel("Load", make_unique<customLoad>());
    builder.registerFactory("ActivePresetComponent", &ActivePresetComponentItem::factory);
    //DBG(builder.getGuiRootNode().toXmlString());
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
void MicrotonalSynthAudioProcessorEditor::loadMicrotonalPreset(int preset) {
    // choose a file
    chooser = std::make_unique<juce::FileChooser>("Load a microtonal mapping preset", juce::File::getSpecialLocation(juce::File::userDocumentsDirectory), "*xml", true, false);
    auto flags = juce::FileBrowserComponent::openMode
        | juce::FileBrowserComponent::canSelectFiles;
    chooser->launchAsync(flags, [this, preset] (const juce::FileChooser& fc) {
        if (fc.getResult() == juce::File{})
            return;
        juce::File myFile;

        myFile = fc.getResult();	
        juce::String fileName = myFile.getFileName();
        microtonalPresetNames[preset] = fileName;

        juce::XmlDocument doc(myFile.loadFileAsString());
        juce::XmlElement config = *doc.getDocumentElement();
        juce::ValueTree t;
        t = t.fromXml(config);
        microtonalMappings[preset].base_frequency = stod(t.getProperty("base_frequency").toString().toStdString());
        microtonalMappings[preset].divisions = stod(t.getProperty("total_divisions").toString().toStdString());
        int i = 0;
        for (juce::ValueTree frequency : t) {
            microtonalMappings[preset].frequencies[i].index = stoi(frequency.getProperty("index").toString().toStdString());
            microtonalMappings[preset].frequencies[i].frequency = stod(frequency.getProperty("value").toString().toStdString());
            i++;
        }
    });

    /* Used for reference */
    /* 
        MemoryBlock data;
		if (fc.getResult().loadFileAsData (data))
			processor->setStateInformation (data.getData(), (int) data.getSize());
		else
			AlertWindow::showMessageBoxAsync (
				AlertWindow::WarningIcon,
				TRANS("Error whilst loading"),
				TRANS("Couldn't read from the specified file!")
			);
    */
}
void MicrotonalSynthAudioProcessorEditor::saveMicrotonalPreset(int preset) {
    // choose a file
    chooser = std::make_unique<juce::FileChooser>("Save a microtonal mapping preset", juce::File::getSpecialLocation(juce::File::userDocumentsDirectory), "*xml", true, false);
    auto flags = juce::FileBrowserComponent::saveMode
        | juce::FileBrowserComponent::canSelectFiles
        | juce::FileBrowserComponent::warnAboutOverwriting;
    juce::String mapping = microtonalMappings[preset].generateValueTree().toXmlString();
	chooser->launchAsync(flags, [this, mapping, preset] (const juce::FileChooser& fc) {
        if (fc.getResult() == juce::File{})
            return;
        juce::File myFile = fc.getResult().withFileExtension("xml");
        juce::String fileName = myFile.getFileName(); 
        microtonalPresetNames[preset] = fileName;
        /* Save file logic goes here*/
        if (!myFile.replaceWithText(mapping)) {
			juce::AlertWindow::showMessageBoxAsync (
				juce::AlertWindow::WarningIcon,
				TRANS("Error whilst saving"),
				TRANS("Couldn't write to the specified file!")
			);
        }
        /* End save file logic*/
    });
}



