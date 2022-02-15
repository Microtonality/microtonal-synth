/*
  ==============================================================================

    MicrotonalMapper.cpp
    Created: 9 Feb 2022 11:10:24am
    Author:  Michael

  ==============================================================================
*/

#include "MicrotonalMapper.h"
#include <math.h>
#include <iostream>
#include <fstream>
using namespace std;
MicrotonalConfig microtonalData;
//==============================================================================
MainContentComponent::MainContentComponent()
    : synthAudioSource(keyboardState),
    keyboardComponent(keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
        keyboardWindow.setColour(juce::TextButton::buttonColourId, juce::Colours::grey);
        keyboardWindow.setEnabled(false);
        keyboardWindow.setColour(juce::ComboBox::outlineColourId, juce::Colours::blue);
        addAndMakeVisible(keyboardWindow);

        divisionInput.setFont(juce::Font(20.0f, juce::Font::bold));
        divisionInput.setText(to_string((int)microtonalData.divisions), juce::dontSendNotification);
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
            else {
                divisionInput.setText(to_string(divisions), juce::dontSendNotification);
            }
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
        baseFreqInput.setText(to_string((int)microtonalData.base_frequency), juce::dontSendNotification);
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
        setSize (1200, 800);
        startTimer (400);
        genFreqFunc();
}

MainContentComponent::~MainContentComponent()
{
    shutdownAudio();
}

void MainContentComponent::resized()
{       
        // Set Main Window
        auto area = getLocalBounds();
        auto upperWindowArea = area.removeFromTop(getHeight());
        upperWindow.setBounds(upperWindowArea);

        // Set Keyboard Window
        auto keyboardWindowHeight = upperWindowArea.getHeight() / 2;
        auto keyboardWindowWidth = upperWindowArea.getWidth() * (6.0/7.0);
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
        keyboardComponent.setKeyWidth(keyboardArea.getWidth() / 7.0);
        keyboardComponent.setAvailableRange(72, 83);

        int boxHeight = divisionInput.getWidth() / 3;
        int boxWidth = boxHeight + 5;
        int maxDivisions = 24;
        generateFrequencies.setBounds(upperWindow.getX(), upperWindow.getY() + (keyboardWindow.getHeight() * 2) / 4 + 10, upperWindow.getWidth() - keyboardWindowWidth + keyboardWindowMargin, frequencyHeight);

        for (int i = 0; i < maxDivisions; i++) {
            int X = upperWindow.getX() + 1.3 * generateFrequencies.getWidth() + ((boxWidth + 2) * i), 
                Y = upperWindow.getY() + (keyboardWindow.getHeight() * 2) / 4 + (generateFrequencies.getHeight() / 3),
                width = boxWidth, 
                height = boxHeight;
            frequencyBoxes[i].setBounds(X, Y, width, height);
            frequencyBoxes[i].getBestWidthForHeight(boxHeight);
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

void MainContentComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate) 
{
    synthAudioSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

float MainContentComponent::roundoff(float value, unsigned char prec)
{
    float pow_10 = pow(10.0f, (float)prec);
    return round(value * pow_10) / pow_10;
}
void MainContentComponent::paint(juce::Graphics& g) {
    for (int i = 0; i < frequencies.size(); i++) {
        for (int j = 0; j < 12; j++) {
            if (frequencies[i] == microtonalData.frequencies[j].frequency) {
                float startX = frequencyBoxes[i].getX() + (frequencyBoxes[i].getWidth() / 2),
                    startY = frequencyBoxes[i].getY() + frequencyBoxes[i].getHeight(),
                    endX = noteButtons[j].getX() + (noteButtons[j].getWidth() / 2),
                    endY = noteButtons[j].getY();

                juce::Line<float> line(juce::Point<float>(startX,startY), juce::Point<float>(endX,endY));
                g.setColour(freqColors[j]);
                g.drawLine(line, 3.0f);
                frequencyBoxes[i].setColour(juce::TextButton::buttonColourId, freqColors[j]);
            }
        }
    }
}
void MainContentComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) 
{
    synthAudioSource.getNextAudioBlock(bufferToFill);
}

void MainContentComponent::releaseResources() 
{
    synthAudioSource.releaseResources();
}

void MainContentComponent::buttonClicked(juce::Button* btn)
{
    for (int i = 0; i < 24; i++) {
        if (btn == &generateFrequencies) {
            for (int i = 0; i < 12; i++) {
                microtonalData.frequencies[i].frequency = NULL;
                microtonalData.frequencies[i].index = NULL;
            }
            genFreqFunc(); 
            return; 
        }
        else if (btn == &frequencyBoxes[i]) {
            if (freqBoxIndex != i) { freqBoxIndex = i; return; }
            
            // logic for changing frequency
        }
        else if (btn == &noteButtons[i]) {
            if (freqBoxIndex == -1) return;
            if (microtonalData.frequencies[i].frequency != NULL) {
                for (int j = 0; j < frequencies.size(); j++) {
                    if (frequencies[j] == microtonalData.frequencies[i].frequency) {
                        frequencyBoxes[j].setColour(juce::TextButton::buttonColourId, juce::Colours::white);
                    }
                }
            }
            microtonalData.frequencies[i].index = freqBoxIndex;
            microtonalData.frequencies[i].frequency = frequencies[freqBoxIndex];
            for (int k = 0; k < 12; k++) {
                if (k == i) continue;
                
                if (microtonalData.frequencies[k].frequency == microtonalData.frequencies[i].frequency) microtonalData.frequencies[k].frequency = NULL;
            }
            noteButtons[i].setColour(juce::TextButton::buttonColourId, freqColors[i]);
            frequencyBoxes[freqBoxIndex].setColour(juce::TextButton::buttonColourId, freqColors[i]);
            freqBoxIndex = -1;
            repaint();
            return;
        }
        else if (btn == &saveToXMLBtn) {
            DBG(writeValuesToXML());
    return;
        }
    }
}

void MainContentComponent::genFreqFunc() {
    microtonalData.divisions = divisionInput.getText().getDoubleValue();
    microtonalData.base_frequency = baseFreqInput.getText().getDoubleValue();
    frequencies = microtonalData.getAllFrequencies();

    for (int i = 0; i < microtonalData.divisions; i++) {
        frequencyBoxes[i].setButtonText(to_string(frequencies[i]).substr(0, 5));
        frequencyBoxes[i].setColour(juce::TextButton::buttonColourId, juce::Colours::white);
        frequencyBoxes[i].setColour(juce::TextButton::textColourOffId, juce::Colours::black);
        frequencyBoxes[i].addListener(this);
        addAndMakeVisible(frequencyBoxes[i]);
    }
    for (int i = 0; i < 12; i++) {
        //selectedFrequencies[i] = NULL;
        noteButtons[i].setColour(juce::TextButton::buttonColourId, freqColors[i]);
        noteButtons[i].setColour(juce::TextButton::textColourOffId, juce::Colours::black);
        noteButtons[i].addListener(this);
    }
    for (int i = microtonalData.divisions; i < 24; i++) {
        try{ 
            frequencyBoxes[i].setVisible(false); 
            frequencyBoxes[i].removeListener(this);
        }
        catch (const std::exception&){}
    }
    repaint();
}

string MainContentComponent::writeValuesToXML() {
    ofstream outf{ "../../Configs/previousState.xml" };
    if (!outf) { return "Error loading config."; }
    juce::String writeToXML = microtonalData.generateXML().toString();
    outf << writeToXML;
    return writeToXML.toStdString();
}

void MainContentComponent::timerCallback()
{
    keyboardComponent.grabKeyboardFocus();
    stopTimer();
}

SynthAudioSource::SynthAudioSource(juce::MidiKeyboardState& keyState)
    : keyboardState(keyState)
{
    for (auto i = 0; i < 4; ++i)                // [1]
        synth.addVoice(new SineWaveVoice());
    synth.addSound(new SineWaveSound());       // [2]
}

void SynthAudioSource::setUsingSineWaveSound()
{
    synth.clearSounds();
}

void SynthAudioSource::prepareToPlay(int /*samplesPerBlockExpected*/, double sampleRate) 
{
    synth.setCurrentPlaybackSampleRate(sampleRate); // [3]
}

void SynthAudioSource::releaseResources() {}

void SynthAudioSource::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    bufferToFill.clearActiveBufferRegion();

    juce::MidiBuffer incomingMidi;
    keyboardState.processNextMidiBuffer(incomingMidi, bufferToFill.startSample,
        bufferToFill.numSamples, true);       // [4]
    synth.renderNextBlock(*bufferToFill.buffer, incomingMidi,
        bufferToFill.startSample, bufferToFill.numSamples); // [5]
}