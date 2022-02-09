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


double total_divisions, base_freq, selectedFrequencies[12];
//class MainContentComponent : public juce::AudioAppComponent,
//    private juce::Timer,
//    public Button::Listener
//{
//public:
//    MainContentComponent()
//        : synthAudioSource(keyboardState),
//        keyboardComponent(keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
//    {
//};
//==============================================================================
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




//==============================================================================
struct SineWaveSound : public juce::SynthesiserSound
{
    SineWaveSound() {}

    bool appliesToNote(int) override { return true; }
    bool appliesToChannel(int) override { return true; }
};

//==============================================================================
struct SineWaveVoice : public juce::SynthesiserVoice
{
    SineWaveVoice() {}

    bool canPlaySound(juce::SynthesiserSound* sound) override
    {
        return dynamic_cast<SineWaveSound*> (sound) != nullptr;
    }

    void startNote(int midiNoteNumber, float velocity,
        juce::SynthesiserSound*, int /*currentPitchWheelPosition*/) override
    {
        currentAngle = 0.0;
        level = velocity * 0.15;
        tailOff = 0.0;

        // auto cyclesPerSecond = juce::MidiMessage::getMidiNoteInHertz (midiNoteNumber);
        double cyclesPerSecond;
        if (selectedFrequencies[midiNoteNumber - 72] == NULL) cyclesPerSecond = base_freq * std::pow(2.0, (midiNoteNumber - 69) / 12.0); //change this for key mapping
        else  cyclesPerSecond = selectedFrequencies[midiNoteNumber - 72]; //change this for key mapping

        auto cyclesPerSample = cyclesPerSecond / getSampleRate();

        angleDelta = cyclesPerSample * 2.0 * juce::MathConstants<double>::pi;
    }

    void stopNote(float /*velocity*/, bool allowTailOff) override
    {
        if (allowTailOff)
        {
            if (tailOff == 0.0)
                tailOff = 1.0;
        }
        else
        {
            clearCurrentNote();
            angleDelta = 0.0;
        }
    }

    void pitchWheelMoved(int) override {}
    void controllerMoved(int, int) override {}

    void renderNextBlock(juce::AudioSampleBuffer& outputBuffer, int startSample, int numSamples) override
    {
        auto amplitude = 0.5;
        if (angleDelta != 0.0)
        {
            if (tailOff > 0.0) // [7]
            {
                while (--numSamples >= 0)
                {
                    //Triangle wave
                    // auto soundval = 2 * fmod(currentAngle, 2 * juce::MathConstants<double>::pi) / juce::MathConstants<double>::pi - 2;
     //                if (soundval > 0) {
     //                    soundval = soundval * -1.0;
     //                }
     //                soundval += 1;

                    // Sawtooth wave
                    // auto soundval = fmod(currentAngle, 2 * juce::MathConstants<double>::pi) / juce::MathConstants<double>::pi - 1;

                    //Square wave
                    auto soundval = fmod(currentAngle, 2 *
                        juce::MathConstants<double>::pi) /
                        juce::MathConstants<double>::pi - 1;;
                    if (soundval > 0.0) {
                        soundval = 1.0;
                    }
                    else {
                        soundval = -1.0;
                    }

                    auto currentSample = (float)
                        (soundval * level * tailOff * amplitude);

                    //Sine wave
                    //auto currentSample = (float) (std::sin (currentAngle) * level * tailOff * amplitude);

                    for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
                        outputBuffer.addSample(i, startSample, currentSample);

                    currentAngle += angleDelta;
                    ++startSample;

                    tailOff *= 0.99; // [8]

                    if (tailOff <= 0.005)
                    {
                        clearCurrentNote(); // [9]

                        angleDelta = 0.0;
                        break;
                    }
                }
            }
            else
            {
                while (--numSamples >= 0) // [6]
                {
                    //Triangle wave
                    // auto soundval = 2 * fmod(currentAngle, 2 * juce::MathConstants<double>::pi) / juce::MathConstants<double>::pi - 2;
     //                if (soundval > 0) {
     //                    soundval = soundval * -1.0;
     //                }
     //                soundval += 1;

                    // Sawtooth wave
                    // auto soundval = fmod(currentAngle, 2 * juce::MathConstants<double>::pi) / juce::MathConstants<double>::pi - 1;

                    //Square wave
                    auto soundval = fmod(currentAngle, 2 *
                        juce::MathConstants<double>::pi) /
                        juce::MathConstants<double>::pi - 1;;
                    if (soundval > 0.0) {
                        soundval = 1.0;
                    }
                    else {
                        soundval = -1.0;
                    }

                    auto currentSample = (float)
                        (soundval * level * amplitude);

                    //Sine wave
                    //auto currentSample = (float) (std::sin (currentAngle) * level * amplitude);

                    for (auto i = outputBuffer.getNumChannels(); --i >= 0;)
                        outputBuffer.addSample(i, startSample, currentSample);

                    currentAngle += angleDelta;
                    ++startSample;
                }
            }
        }
    }

private:
    double currentAngle = 0.0, angleDelta = 0.0, level = 0.0, tailOff = 0.0;
};

//==============================================================================
class SynthAudioSource : public juce::AudioSource
{
public:
    SynthAudioSource(juce::MidiKeyboardState& keyState)
        : keyboardState(keyState)
    {
        for (auto i = 0; i < 4; ++i)                // [1]
            synth.addVoice(new SineWaveVoice());

        synth.addSound(new SineWaveSound());       // [2]
    }

    void setUsingSineWaveSound()
    {
        synth.clearSounds();
    }

    void prepareToPlay(int /*samplesPerBlockExpected*/, double sampleRate) override
    {
        synth.setCurrentPlaybackSampleRate(sampleRate); // [3]
    }

    void releaseResources() override {}

    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override
    {
        bufferToFill.clearActiveBufferRegion();

        juce::MidiBuffer incomingMidi;
        keyboardState.processNextMidiBuffer(incomingMidi, bufferToFill.startSample,
            bufferToFill.numSamples, true);       // [4]

        synth.renderNextBlock(*bufferToFill.buffer, incomingMidi,
            bufferToFill.startSample, bufferToFill.numSamples); // [5]
    }

private:
    juce::MidiKeyboardState& keyboardState;
    juce::Synthesiser synth;
};

//==============================================================================
class MainContentComponent : public juce::AudioAppComponent,
    private juce::Timer,
    public juce::Button::Listener
{
public:
    MainContentComponent()
        : synthAudioSource(keyboardState),
        keyboardComponent(keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
    {
        juce::Array<juce::Colour> colours{ juce::Colour(49,49,49),   // background color
                                    juce::Colour(87,87,87),   // sidebar button background
                                    juce::Colour(120,120,120), // input background color
                                    juce::Colour(19,243,67) // input outline and text color
        };
        enum coloursEnum {
            backgroundColor,
            sidebarBtnBackgroundColor,
            inputBackgroundColor,
            inputOutlineTextColor
        };



        upperWindow.setColour(juce::TextButton::buttonColourId, colours[backgroundColor]);
        addAndMakeVisible(upperWindow);

        keyboardWindow.setColour(juce::TextButton::buttonColourId, juce::Colours::grey);
        keyboardWindow.setColour(juce::ComboBox::outlineColourId, juce::Colours::blue);
        addAndMakeVisible(keyboardWindow);

        divisionInput.setFont(juce::Font(20.0f, juce::Font::bold));
        divisionInput.setText(to_string(divisions), juce::dontSendNotification);
        divisionInput.setColour(juce::Label::textColourId, juce::Colours::black);
        divisionInput.setJustificationType(juce::Justification::centred);
        divisionInput.setEditable(true);
        divisionInput.setColour(juce::Label::backgroundColourId, colours[inputBackgroundColor]);
        divisionInput.setColour(juce::Label::outlineColourId, colours[inputOutlineTextColor]);
        divisionInput.onTextChange = [this] {

            if (all_of(divisionInput.getText().begin(), divisionInput.getText().end(), isdigit) && divisionInput.getText().getIntValue() >= 12
                && divisionInput.getText().getIntValue() <= 24) {
                divisionInput.setText(divisionInput.getText(), juce::sendNotification);
                divisions = divisionInput.getText().getIntValue();
            }
            else
                divisionInput.setText(to_string(divisions), juce::dontSendNotification);
        };
        addAndMakeVisible(divisionInput);

        divisionLabel.setFont(juce::Font(20.0f, juce::Font::bold));
        divisionLabel.setText("Notes per octave:", juce::dontSendNotification);
        divisionLabel.attachToComponent(&divisionInput, true);
        divisionLabel.setColour(juce::Label::textColourId, colours[3]);
        divisionLabel.setColour(juce::Label::backgroundColourId, colours[backgroundColor]);
        divisionLabel.setColour(juce::Label::outlineColourId, colours[backgroundColor]);
        divisionLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(divisionLabel);

        baseFreqInput.setFont(juce::Font(18.0f, juce::Font::bold));
        baseFreqInput.setText(to_string((int)frequency), juce::dontSendNotification);
        baseFreqInput.setColour(juce::Label::textColourId, juce::Colours::black);
        baseFreqInput.setColour(juce::Label::outlineColourId, colours[inputOutlineTextColor]);
        baseFreqInput.setColour(juce::Label::backgroundColourId, colours[inputBackgroundColor]);
        baseFreqInput.setJustificationType(juce::Justification::centred);
        baseFreqInput.setEditable(true);
        addAndMakeVisible(baseFreqInput);

        baseFreqLabel.setFont(juce::Font(20.0f, juce::Font::bold));
        baseFreqLabel.setText("Base Frequency:", juce::dontSendNotification);
        baseFreqLabel.attachToComponent(&baseFreqInput, true);
        baseFreqLabel.setColour(juce::Label::textColourId, colours[inputOutlineTextColor]);
        baseFreqLabel.setColour(juce::Label::backgroundColourId, colours[backgroundColor]);
        baseFreqLabel.setColour(juce::Label::outlineColourId, colours[backgroundColor]);
        baseFreqLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(baseFreqLabel);

        generateFrequencies.setColour(juce::TextButton::buttonColourId, colours[inputBackgroundColor]);
        generateFrequencies.setColour(juce::TextButton::textColourOffId, colours[inputOutlineTextColor]);
        generateFrequencies.setButtonText("Generate");
        generateFrequencies.addListener(this);
        addAndMakeVisible(generateFrequencies);



        addAndMakeVisible(keyboardComponent);
        setAudioChannels(0, 2);
        for (auto& btn : noteButtons) {
            btn.setColour(juce::TextButton::buttonColourId, juce::Colours::blue);
            addAndMakeVisible(btn);
        }

        setSize(1200, 800);
        startTimer(400);
    }

    ~MainContentComponent() override
    {
        shutdownAudio();
    }

    void resized() override
    {
        auto area = getLocalBounds();
        auto headerFooterHeight = 36;


        auto sideItemHeight = 40;
        auto sideItemMargin = 5;

        // Set Upper Window
        auto upperWindowArea = area.removeFromTop(getHeight());//.reduced(windowMargin);
        upperWindow.setBounds(upperWindowArea);

        // Set Keyboard Window
        auto keyboardWindowHeight = upperWindowArea.getHeight() / 2;
        auto keyboardWindowWidth = upperWindowArea.getWidth() * (6.0 / 7.0);
        auto keyboardWindowMargin = 10;
        auto keyboardWindowArea = upperWindowArea.removeFromBottom(keyboardWindowHeight).removeFromRight(keyboardWindowWidth).reduced(keyboardWindowMargin);
        keyboardWindow.setBounds(keyboardWindowArea);

        // Set division label
        auto divisionHeight = keyboardWindow.getHeight() / 4;
        auto divisionWidth = keyboardWindow.getWidth() / 10;
        divisionInput.setBounds(keyboardWindow.getX(), upperWindow.getY() + keyboardWindow.getHeight() / 2 - divisionHeight, divisionWidth, divisionHeight);//divisionInput.setBounds(keyboardWindow.getX(), keyboardWindow.getY(), divisionWidth, divisionHeight);
        divisionLabel.setBounds(upperWindow.getX(), upperWindow.getY() + keyboardWindow.getHeight() / 2 - divisionHeight, upperWindow.getWidth() - keyboardWindowWidth + keyboardWindowMargin, divisionHeight);

        // Set frequency label
        auto frequencyHeight = divisionHeight;
        auto frequencyWidth = divisionWidth;
        baseFreqInput.setBounds(keyboardWindow.getX(), upperWindow.getY(), frequencyWidth, frequencyHeight);//baseFreqInput.setBounds(keyboardWindow.getX(), keyboardWindow.getY() + keyboardWindow.getHeight() - frequencyHeight, frequencyWidth, frequencyHeight);//baseFreqInput.setBounds(upperWindow.getX() + 200, upperWindow.getY() + 80, frequencyWidth, frequencyHeight);
        baseFreqLabel.setBounds(upperWindow.getX(), upperWindow.getY(), upperWindow.getWidth() - keyboardWindowWidth + keyboardWindowMargin, frequencyHeight);

        // Set Keyboard
        auto keyboardMargin = 10;
        auto keyboardWidthDelta = 0.2 * keyboardWindowArea.getWidth();
        auto keyboardHeightDelta = 0.1 * keyboardWindowArea.getHeight();
        auto keyboardArea = keyboardWindowArea.reduced(keyboardWidthDelta, keyboardHeightDelta);//.removeFromLeft(keyboardWidth);
        keyboardComponent.setBounds(keyboardArea);


        // Set basefreq 
       // baseFreqInput.setBounds(upperWindowArea.getWidth() / 4, upperWindowArea.getHeight() * (95.0 / 100), 50, 20);


        keyboardComponent.setKeyWidth(keyboardArea.getWidth() / 8.0);
        keyboardComponent.setAvailableRange(72, 84);

        int boxWidth = divisionInput.getWidth() / 3;
        int maxDivisions = 24;
        generateFrequencies.setBounds(upperWindow.getX(), upperWindow.getY() + (keyboardWindow.getHeight() * 2) / 4 + 10, upperWindow.getWidth() - keyboardWindowWidth + keyboardWindowMargin, frequencyHeight);
        for (int i = 0; i < maxDivisions; i++) {
            int X = upperWindow.getX() + 1.5 * generateFrequencies.getWidth() + ((boxWidth + 2) * i),
                Y = upperWindow.getY() + (keyboardWindow.getHeight() * 2) / 4 + (generateFrequencies.getHeight() / 3),
                width = boxWidth,
                height = boxWidth;
            frequencyBoxes[i].setBounds(X, Y, width, height);
        }

        for (int i = 0; i < 12; i++) {
            int boxWidth = (divisionInput.getWidth() / 3) + 2;
            int X = keyboardComponent.getKeyStartPosition(startKey) + keyboardComponent.getX() + ((keyboardComponent.getKeyWidth() / 5) * 3 * i),
                Y = keyboardComponent.getY() - 35,
                width = (boxWidth / 3) * 2,
                height = width;
            noteButtons[i].setBounds(X, Y, width, height);

        }

    }

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override
    {
        synthAudioSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
    }

    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override
    {
        synthAudioSource.getNextAudioBlock(bufferToFill);
    }

    void releaseResources() override
    {
        synthAudioSource.releaseResources();
    }

    void buttonClicked(juce::Button* btn) override
    {
        for (int i = 0; i < 24; i++) {
            if (btn == &generateFrequencies) { genFreqFunc(); return; }
            else if (btn == &frequencyBoxes[i]) {
                freqBoxIndex = i;
                return;
            }
            else if (btn == &noteButtons[i]) {
                if (freqBoxIndex == -1) return;
                if (selectedFrequencies[i] != NULL) {
                    for (int j = 0; j < frequencies.size(); j++) {
                        if (frequencies[j] == selectedFrequencies[i]) {
                            frequencyBoxes[j].setColour(juce::TextButton::buttonColourId, juce::Colours::white);
                        }
                    }
                }
                selectedFrequencies[i] = frequencies[freqBoxIndex];
                for (int k = 0; k < 12; k++) {
                    if (k == i) continue;

                    if (selectedFrequencies[k] == selectedFrequencies[i]) selectedFrequencies[k] = NULL;
                }
                noteButtons[i].setColour(juce::TextButton::buttonColourId, freqColors[i]);
                frequencyBoxes[freqBoxIndex].setColour(juce::TextButton::buttonColourId, freqColors[i]);
                freqBoxIndex = -1;
                return;
            }
        }
    }
    void genFreqFunc() {
        total_divisions = divisionInput.getText().getDoubleValue();
        base_freq = baseFreqInput.getText().getDoubleValue();
        frequencies.clear(); // Dynamic array to store frequencies
        for (int i = 0; i <= total_divisions; i++) {
            double step_calc = (i / total_divisions);
            frequencies.push_back(base_freq * pow(2, step_calc));
        }
        for (int i = 0; i < total_divisions; i++) {
            frequencyBoxes[i].setButtonText(to_string(frequencies[i]).substr(0, to_string(frequencies[i]).find(".")));
            frequencyBoxes[i].setColour(juce::TextButton::buttonColourId, juce::Colours::white);
            frequencyBoxes[i].setColour(juce::TextButton::textColourOffId, juce::Colours::black);
            frequencyBoxes[i].addListener(this);
            addAndMakeVisible(frequencyBoxes[i]);
        }
        for (int i = 0; i < 12; i++) {
            selectedFrequencies[i] = NULL;
            noteButtons[i].setColour(juce::TextButton::buttonColourId, freqColors[i]);
            noteButtons[i].setColour(juce::TextButton::textColourOffId, juce::Colours::black);
            noteButtons[i].addListener(this);
        }
        for (int i = total_divisions; i < 24; i++) {
            try {
                frequencyBoxes[i].setVisible(false);
                frequencyBoxes[i].removeListener(this);
            }
            catch (const std::exception&) {}
        }
    }
private:
    void timerCallback() override
    {
        keyboardComponent.grabKeyboardFocus();
        stopTimer();
    }


    int test = 1;
    int divisions = 12;
    double frequency = 440.0;
    //==========================================================================
    juce::MidiKeyboardState keyboardState;
    SynthAudioSource synthAudioSource;

    juce::MidiKeyboardComponent keyboardComponent;

    juce::TextButton keyboardWindow;
    juce::TextButton upperWindow;

    juce::Label baseFreqLabel;
    juce::Label divisionLabel;
    juce::Label baseFreqInput;
    juce::Label divisionInput;
    int freqBoxIndex = -1, selectedFrequencyIndex = 0;
    vector<double> frequencies;


    juce::Array<juce::Colour> freqColors{
        juce::Colour(254,0,0),
        juce::Colour(232,79,0),
        juce::Colour(254,153,1),
        juce::Colour(255,204,0),
        juce::Colour(190,233,0),
        juce::Colour(102,204,0),
        juce::Colour(0,153,0),
        juce::Colour(10,180,195),
        juce::Colour(0,81,212),
        juce::Colour(103,0,153),
        juce::Colour(153,0,153),
        juce::Colour(204,0,152)
    };

    juce::TextButton generateFrequencies;
    int startKey = 72;
    juce::TextButton frequencyBoxes[24];
    juce::TextButton noteButtons[12];

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainContentComponent)
};



class MainContentComponentItem : public foleys::GuiItem
{
public:
    FOLEYS_DECLARE_GUI_FACTORY(MainContentComponentItem)

        MainContentComponentItem(foleys::MagicGUIBuilder& builder, const juce::ValueTree& node) : foleys::GuiItem(builder, node)
    {

        addAndMakeVisible(maincontentcomponent);
    }

    std::vector<foleys::SettableProperty> getSettableProperties() const override
    {
        std::vector<foleys::SettableProperty> newProperties;

        newProperties.push_back({ configNode, "factor", foleys::SettableProperty::Number, 1.0f, {} });

        return newProperties;
    }

    // Override update() to set the values to your custom component
    void update() override
    {
        auto factor = getProperty("factor");
    }

    juce::Component* getWrappedComponent() override
    {
        return &maincontentcomponent;
    }

private:
    MainContentComponent maincontentcomponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainContentComponentItem)
};

class MicrotonalButton : public foleys::GuiItem
{
public:
    FOLEYS_DECLARE_GUI_FACTORY(MicrotonalButton)

    static const juce::Identifier pText;
    static const juce::Identifier pOnClick;

    MicrotonalButton(foleys::MagicGUIBuilder& builder, const juce::ValueTree& node) : GuiItem(builder, node)
    {
        setColourTranslation(
            {
                { "button-color", juce::TextButton::buttonColourId },
                { "button-on-color", juce::TextButton::buttonOnColourId },
                { "button-off-text", juce::TextButton::textColourOffId },
                { "button-on-text", juce::TextButton::textColourOnId }
            });

        addAndMakeVisible(button);
    }

    void update() override
    {
        attachment.reset();

        auto parameter = configNode.getProperty(foleys::IDs::parameter, juce::String()).toString();
        if (parameter.isNotEmpty())
            attachment = getMagicState().createAttachment(parameter, button);

        button.setClickingTogglesState(parameter.isNotEmpty());
        button.setButtonText(magicBuilder.getStyleProperty(pText, configNode));

        auto triggerID = getProperty(pOnClick).toString();
        if (triggerID.isNotEmpty())
            button.onClick = getMagicState().getTrigger(triggerID);
    }

    std::vector<foleys::SettableProperty> getSettableProperties() const override
    {
        std::vector<foleys::SettableProperty> props;

        props.push_back({ configNode, foleys::IDs::parameter, foleys::SettableProperty::Choice, {}, magicBuilder.createParameterMenuLambda() });
        props.push_back({ configNode, pText, foleys::SettableProperty::Text, {}, {} });
        props.push_back({ configNode, pOnClick, foleys::SettableProperty::Choice, {}, magicBuilder.createTriggerMenuLambda() });

        return props;
    }

    juce::Component* getWrappedComponent() override
    {
        return &button;
    }

private:
    juce::TextButton button;
    std::unique_ptr<juce::ButtonParameterAttachment> attachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MicrotonalButton)
};
const juce::Identifier MicrotonalButton::pText{ "text" };
const juce::Identifier MicrotonalButton::pOnClick{ "onClick" };

juce::AudioProcessorEditor* MicrotonalSynthAudioProcessorEditor::createEditor()
{
    // MAGIC GUI: we create our custom builder instance here, that will be available for all factories we add
    auto builder = std::make_unique<foleys::MagicGUIBuilder>(magicState);
    builder->registerJUCEFactories();

    builder->registerFactory("MainContentComponentItem", &MainContentComponentItem::factory);
    builder->registerFactory("MicrotonalConfigButton", &MicrotonalButton::factory);

    return new foleys::MagicPluginEditor(magicState, std::move(builder));
}

