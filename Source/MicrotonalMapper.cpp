/*
  ==============================================================================

    MicrotonalMapper.cpp
    Created: 9 Feb 2022 11:10:24am
    Author:  Michael

  ==============================================================================
*/

#include "MicrotonalMapper.h"
double total_divisions, base_freq, selectedFrequencies[12];
//==============================================================================
MainContentComponent::MainContentComponent()
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

MainContentComponent::~MainContentComponent()
{
    shutdownAudio();
}

void MainContentComponent::resized()
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

void MainContentComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate) 
{
    synthAudioSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
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

void MainContentComponent::genFreqFunc() {
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