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
MicrotonalConfig microtonalMappings[7];
int mappingIndex;
//==============================================================================
MainContentComponent::MainContentComponent(int index)
    : synthAudioSource(keyboardState),
    keyboardComponent(keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
        divisions = (int)microtonalMappings[mappingIndex].divisions;
        mappingIndex = index;
        keyboardWindow.setColour(juce::TextButton::buttonColourId, juce::Colours::grey);
        keyboardWindow.setEnabled(false);
        keyboardWindow.setColour(juce::ComboBox::outlineColourId, juce::Colours::blue);
        addAndMakeVisible(keyboardWindow);

        divisionInput.setFont(juce::Font(20.0f, juce::Font::bold));
        divisionInput.setText(to_string((int)microtonalMappings[mappingIndex].divisions), juce::dontSendNotification);
        divisionInput.setColour(juce::Label::textColourId, juce::Colours::black);
        divisionInput.setJustificationType(juce::Justification::centred);
        divisionInput.setEditable(true);
        divisionInput.setColour(juce::Label::backgroundColourId, colours[inputBackgroundColor]);
        divisionInput.setColour(juce::Label::outlineColourId, colours[inputOutlineTextColor]);
        divisionInput.onTextChange = [this] { 
            if (validateDivisionInput(divisionInput.getText())) {
                divisionInput.setText(divisionInput.getText(), juce::sendNotification);
                divisions = divisionInput.getText().getIntValue();
            }
            else {
                divisionInput.setText(juce::String(divisions), juce::dontSendNotification);
            }
        };
        divisionInput.setMouseCursor(juce::MouseCursor::IBeamCursor);
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
        baseFreqInput.setText(to_string((int)microtonalMappings[mappingIndex].base_frequency), juce::dontSendNotification);
        baseFreqInput.setColour(juce::Label::textColourId, juce::Colours::black);
        baseFreqInput.setColour(juce::Label::outlineColourId, colours[inputOutlineTextColor]);
        baseFreqInput.setColour(juce::Label::backgroundColourId, colours[inputBackgroundColor]);
        baseFreqInput.setJustificationType(juce::Justification::centred);
        baseFreqInput.setEditable(true);
        baseFreqInput.onTextChange = [this] {
            if (validateFrequencyInput(baseFreqInput.getText())) {
                baseFreqInput.setText(baseFreqInput.getText(), juce::sendNotification);
                frequency = baseFreqInput.getText().getDoubleValue();
            }
            else {
                baseFreqInput.setText(juce::String(frequency), juce::dontSendNotification);
            }
        };
        baseFreqInput.setMouseCursor(juce::MouseCursor::IBeamCursor);
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
        generateFrequencies.setMouseCursor(juce::MouseCursor::PointingHandCursor);
        addAndMakeVisible(generateFrequencies);

        addAndMakeVisible(keyboardComponent);
        setAudioChannels(0, 2);
        for (auto& btn : noteButtons) {
            btn.setColour(juce::TextButton::buttonColourId, juce::Colours::blue);
            btn.setMouseCursor(juce::MouseCursor::PointingHandCursor);
            addAndMakeVisible(btn);
        }
        setSize (1200, 800);
        startTimer (400);
        genFreqFunc();
        // space separated "start finish increment"

        shortHandInput.setFont(juce::Font(20.0f, juce::Font::bold));
        shortHandInput.setColour(juce::Label::textColourId, juce::Colours::black);
        shortHandInput.setJustificationType(juce::Justification::centred);
        shortHandInput.setEditable(true);
        shortHandInput.setColour(juce::Label::backgroundColourId, colours[inputBackgroundColor]);
        shortHandInput.setColour(juce::Label::outlineColourId, colours[inputOutlineTextColor]);
        shortHandInput.setMouseCursor(juce::MouseCursor::IBeamCursor);

        addAndMakeVisible(shortHandInput);
        shortHandBtn.setColour(juce::TextButton::buttonColourId, colours[inputBackgroundColor]);
        shortHandBtn.setColour(juce::TextButton::textColourOffId, colours[inputOutlineTextColor]);
        shortHandBtn.setButtonText("Quick Map");
        shortHandBtn.addListener(this);        
        shortHandBtn.setMouseCursor(juce::MouseCursor::PointingHandCursor);
        addAndMakeVisible(shortHandBtn);
}

MainContentComponent::~MainContentComponent()
{
    shutdownAudio();
}

void MainContentComponent::resized()
{       
        // Set Main Window
        divisions = (int)microtonalMappings[mappingIndex].divisions;
        auto area = getLocalBounds();
        auto upperWindowArea = area.removeFromTop(getHeight());
        upperWindow.setBounds(upperWindowArea);

        // Set Keyboard Window
        auto keyboardWindowHeight = upperWindowArea.getHeight() * 0.6;
        auto keyboardWindowWidth = upperWindowArea.getWidth() * (6.0/7.0);
        auto keyboardWindowMargin = 10;
        auto keyboardWindowArea = upperWindowArea.removeFromBottom(keyboardWindowHeight).reduced(keyboardWindowMargin);
        keyboardWindow.setBounds(keyboardWindowArea);


        // Set Keyboard
        auto keyboardMargin = 10;
        auto keyboardWidthDelta = 0.2 * keyboardWindowArea.getWidth();
        auto keyboardHeightDelta = 0.1 * keyboardWindowArea.getHeight();
        auto keyboardArea = keyboardWindowArea.reduced(keyboardWidthDelta, keyboardHeightDelta);//.removeFromLeft(keyboardWidth);
        keyboardComponent.setBounds(keyboardArea);
        keyboardComponent.setKeyWidth(keyboardArea.getWidth() / 7.0);
        keyboardComponent.setAvailableRange(72, 83);

        // Set division label
        auto divisionHeight = keyboardWindow.getHeight() / 4;
        auto divisionWidth = keyboardWindow.getWidth() / 10;
        auto divisionMargin = 10;
        divisionInput.setBounds(keyboardWindow.getX() + divisionWidth + divisionMargin, keyboardWindow.getY() + divisionMargin, divisionWidth / 2, divisionHeight);//divisionInput.setBounds(keyboardWindow.getX(), keyboardWindow.getY(), divisionWidth, divisionHeight);
        divisionLabel.setBounds(keyboardWindow.getX() + divisionMargin, keyboardWindow.getY() + divisionMargin, divisionWidth, divisionHeight);
        

        // Set frequency label
        auto frequencyHeight = divisionHeight;
        auto frequencyWidth = divisionWidth;
        baseFreqInput.setBounds(keyboardWindow.getX() + frequencyWidth + divisionMargin, keyboardWindow.getY() + frequencyHeight + divisionMargin, frequencyWidth / 2, frequencyHeight);//baseFreqInput.setBounds(keyboardWindow.getX(), keyboardWindow.getY() + keyboardWindow.getHeight() - frequencyHeight, frequencyWidth, frequencyHeight);//baseFreqInput.setBounds(upperWindow.getX() + 200, upperWindow.getY() + 80, frequencyWidth, frequencyHeight);
        baseFreqLabel.setBounds(keyboardWindow.getX() + divisionMargin, keyboardWindow.getY() + frequencyHeight + divisionMargin, frequencyWidth, frequencyHeight);


        int boxHeight = divisionInput.getWidth()*0.70;
        int boxWidth = boxHeight + 5;
        int maxDivisions = 24;
        generateFrequencies.setBounds(keyboardWindow.getX() + divisionMargin, keyboardWindow.getY() + 2*frequencyHeight + divisionMargin, (3.0/2)*frequencyWidth, frequencyHeight);
        shortHandBtn.setBounds(((keyboardWindow.getWidth() + 2 * keyboardWindowMargin) + (keyboardComponent.getX() + keyboardComponent.getWidth()))/2 - (3.0 / 4) * frequencyWidth, keyboardWindow.getY() + 2 * frequencyHeight + divisionMargin, (3.0 / 2) * frequencyWidth, frequencyHeight);
        shortHandInput.setBounds(((keyboardWindow.getWidth() + 2 * keyboardWindowMargin) + (keyboardComponent.getX() + keyboardComponent.getWidth())) / 2 - (3.0 / 4) * frequencyWidth, keyboardWindow.getY() + 2 * frequencyHeight + divisionMargin - frequencyHeight/2, (3.0 / 2) * frequencyWidth, frequencyHeight / 2);
        auto iter = 1;
        vector<double>positions;
        positions.push_back(keyboardWindow.getWidth() / 2 + boxWidth / 2 - 10);
        for (int i = 1; i < divisions; i++) {
            positions.push_back(positions[0] + iter * (i % 2 == 0 ? 1 : -1) * ((boxWidth + 2)));
            iter = i % 2 == 0 ? iter + 1 : iter;
        }
        sort(positions.begin(), positions.end());
        auto Y = upperWindow.getY() + (keyboardWindow.getHeight() * 2) / 10; //+ (generateFrequencies.getHeight() / 3);
        for (int i = 0; i < divisions; i++) {
            frequencyBoxes[i].setBounds(positions[i], Y, boxWidth, boxHeight);
            frequencyBoxes[i].getBestWidthForHeight(boxHeight);
             //X = startX + iter * (i % 2 == 0 ? 1 : -1) * ((boxWidth + 2));
             //iter = i % 2 == 0 ? iter + 1 : iter;
             //frequencyBoxes[i].setBounds(X, Y, boxWidth, boxHeight);
             //frequencyBoxes[i].getBestWidthForHeight(boxHeight);

        }
        for (int i = 0; i < 12; i++) {
            int boxWidth = (divisionInput.getWidth() *0.7) + 2;
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
            if (roundoff(frequencies[i], 1) == roundoff(microtonalMappings[mappingIndex].frequencies[j].frequency, 1)) {
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

void MainContentComponent::undoButtonHighlighting()
{
    for (auto& b : frequencyBoxes) {
        b.removeColour(juce::ComboBox::outlineColourId);
        b.setColour(juce::TextButton::buttonColourId, juce::Colours::white);
    }
}

void MainContentComponent::buttonClicked(juce::Button* btn)
{
    for (int i = 0; i < 24; i++) {
        if (btn == &generateFrequencies) {
            for (int i = 0; i < 12; i++) {
                microtonalMappings[mappingIndex].frequencies[i].frequency = NULL;
                microtonalMappings[mappingIndex].frequencies[i].index = NULL;
            }
            genFreqFunc(); 
            this->resized();
            return; 
        }
        else if (btn == &frequencyBoxes[i]) {
            if (freqBoxIndex != i) { 
                freqBoxIndex = i; undoButtonHighlighting(); 
                frequencyBoxes[i].setColour(juce::TextButton::buttonColourId, juce::Colours::yellow);
                frequencyBoxes[i].setColour(juce::ComboBox::outlineColourId, juce::Colours::black); 
                return; 
            }

        }
        else if (btn == &noteButtons[i]) {
            if (freqBoxIndex == -1) return;
            if (microtonalMappings[mappingIndex].frequencies[i].frequency != NULL) {
                for (int j = 0; j < frequencies.size(); j++) {
                    if (frequencies[j] == microtonalMappings[mappingIndex].frequencies[i].frequency) {
                        frequencyBoxes[j].setColour(juce::TextButton::buttonColourId, juce::Colours::white);
                    }
                }
            }
            microtonalMappings[mappingIndex].frequencies[i].index = freqBoxIndex;
            microtonalMappings[mappingIndex].frequencies[i].frequency = frequencies[freqBoxIndex];
            for (int k = 0; k < 12; k++) {
                if (k == i) continue;
                
                if (microtonalMappings[mappingIndex].frequencies[k].frequency == microtonalMappings[mappingIndex].frequencies[i].frequency) microtonalMappings[mappingIndex].frequencies[k].frequency = NULL;
            }
            noteButtons[i].setColour(juce::TextButton::buttonColourId, freqColors[i]);
            frequencyBoxes[freqBoxIndex].setColour(juce::TextButton::buttonColourId, freqColors[i]);
            frequencyBoxes[freqBoxIndex].removeColour(juce::ComboBox::outlineColourId);
            freqBoxIndex = -1;
            repaint();
            return;
        }
        else if (btn == &saveToXMLBtn) {
            DBG(writeValuesToXML());
            return;
        }
        else if (btn == &shortHandBtn) {
            mappingShortcut(shortHandInput.getText().toStdString());
        }
    }
}
void MainContentComponent::mappingShortcut(string inputString) {
    if (inputString == "") return;
    istringstream iss(inputString);

    string word;
    //Extract each words only..no spaces.
    //This way it can handle any
    //special characters.
    iss >> word;
    int start = stoi(word);
    iss >> word;
    int finish = stoi(word);
    iss >> word;
    int steps = stoi(word);
    int noteBlock = 0;
    genFreqFunc();
    for (int i = 0; i < 12; i++) {
        microtonalMappings[mappingIndex].frequencies[i].index = NULL;
        microtonalMappings[mappingIndex].frequencies[i].frequency = NULL;
    }
    for (int i = start; i <= finish; i += steps, noteBlock++) {
        if (i >= microtonalMappings[mappingIndex].divisions) break;
        microtonalMappings[mappingIndex].frequencies[noteBlock].index = noteBlock;
        microtonalMappings[mappingIndex].frequencies[noteBlock].frequency = frequencies[i];
    }
    repaint();
}
void MainContentComponent::genFreqFunc() {
    microtonalMappings[mappingIndex].divisions = divisionInput.getText().getDoubleValue();
    microtonalMappings[mappingIndex].base_frequency = baseFreqInput.getText().getDoubleValue();
    frequencies = microtonalMappings[mappingIndex].getAllFrequencies();

    for (int i = 0; i < microtonalMappings[mappingIndex].divisions; i++) {
        frequencyBoxes[i].setButtonText(to_string(frequencies[i]).substr(0, 5));
        frequencyBoxes[i].setColour(juce::TextButton::buttonColourId, juce::Colours::white);
        frequencyBoxes[i].setColour(juce::TextButton::textColourOffId, juce::Colours::black);
        frequencyBoxes[i].addListener(this);
        frequencyBoxes[i].setMouseCursor(juce::MouseCursor::PointingHandCursor);
        addAndMakeVisible(frequencyBoxes[i]);
    } 
    for (int i = 0; i < 12; i++) {
        //selectedFrequencies[i] = NULL;
        noteButtons[i].setColour(juce::TextButton::buttonColourId, freqColors[i]);
        noteButtons[i].setColour(juce::TextButton::textColourOffId, juce::Colours::black);
        noteButtons[i].addListener(this);
    }
    for (int i = microtonalMappings[mappingIndex].divisions; i < 24; i++) {
        try{ 
            frequencyBoxes[i].setMouseCursor(juce::MouseCursor::NormalCursor);
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
    // juce::String writeToXML = microtonalMappings[mappingIndex].generateXML().toString();
    // outf << writeToXML;
    return NULL;
}

bool MainContentComponent::validateDivisionInput(const juce::String& s)
{
    return s.containsOnly("0123456789") && s.getIntValue() >= 12 && s.getIntValue() <= 24;
}

bool MainContentComponent::validateFrequencyInput(const juce::String& s)
{
    return s.getDoubleValue() >= 1 && s.getDoubleValue() <= 50000;
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