/*
  ==============================================================================

   This file is part of the JUCE tutorials.
   Copyright (c) 2020 - Raw Material Software Limited

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES,
   WHETHER EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR
   PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             MicrotonalKeyboardComponent
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Synthesiser with midi input.

 dependencies:     juce_audio_basics, juce_audio_devices, juce_audio_formats,
                   juce_audio_processors, juce_audio_utils, juce_core,
                   juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics, juce_gui_extra
 exporters:        xcode_mac, vs2019, linux_make

 type:             Component
 mainClass:        MainContentComponent

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/


#pragma once
#include <string> 
#include <cctype> 
#include <math.h>
#include <iostream>
#include <algorithm>
using namespace std;
//==============================================================================
struct SineWaveSound   : public juce::SynthesiserSound
{
    SineWaveSound() {}

    bool appliesToNote    (int) override        { return true; }
    bool appliesToChannel (int) override        { return true; }
};

//==============================================================================
struct SineWaveVoice   : public juce::SynthesiserVoice
{
    SineWaveVoice() {}

    bool canPlaySound (juce::SynthesiserSound* sound) override
    {
        return dynamic_cast<SineWaveSound*> (sound) != nullptr;
    }

    void startNote (int midiNoteNumber, float velocity,
                    juce::SynthesiserSound*, int /*currentPitchWheelPosition*/) override
    {
        currentAngle = 0.0;
        level = velocity * 0.15;
        tailOff = 0.0;

        //auto cyclesPerSecond = juce::MidiMessage::getMidiNoteInHertz (midiNoteNumber);
        auto cyclesPerSecond = 440 * std::pow(2.0, (midiNoteNumber - 69) / 12.0); //change this for key mapping
        auto cyclesPerSample = cyclesPerSecond / getSampleRate();

        angleDelta = cyclesPerSample * 2.0 * juce::MathConstants<double>::pi;
    }

    void stopNote (float /*velocity*/, bool allowTailOff) override
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

    void pitchWheelMoved (int) override      {}
    void controllerMoved (int, int) override {}

    void renderNextBlock (juce::AudioSampleBuffer& outputBuffer, int startSample, int numSamples) override
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
                        outputBuffer.addSample (i, startSample, currentSample);

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
                        outputBuffer.addSample (i, startSample, currentSample);

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
class SynthAudioSource   : public juce::AudioSource
{
public:
    SynthAudioSource (juce::MidiKeyboardState& keyState)
        : keyboardState (keyState)
    {
        for (auto i = 0; i < 4; ++i)                // [1]
            synth.addVoice (new SineWaveVoice());

        synth.addSound (new SineWaveSound());       // [2]
    }

    void setUsingSineWaveSound()
    {
        synth.clearSounds();
    }

    void prepareToPlay (int /*samplesPerBlockExpected*/, double sampleRate) override
    {
        synth.setCurrentPlaybackSampleRate (sampleRate); // [3]
    }

    void releaseResources() override {}

    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override
    {
        bufferToFill.clearActiveBufferRegion();

        juce::MidiBuffer incomingMidi;
        keyboardState.processNextMidiBuffer (incomingMidi, bufferToFill.startSample,
                                             bufferToFill.numSamples, true);       // [4]

        synth.renderNextBlock (*bufferToFill.buffer, incomingMidi,
                               bufferToFill.startSample, bufferToFill.numSamples); // [5]
    }

private:
    juce::MidiKeyboardState& keyboardState;
    juce::Synthesiser synth;
};

//==============================================================================
class MainContentComponent   : public juce::AudioAppComponent,
                               private juce::Timer
{
public:
    MainContentComponent()
        : synthAudioSource  (keyboardState),
          keyboardComponent (keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
    {
     
        header.setColour(juce::TextButton::buttonColourId, juce::Colours::cornflowerblue);
        header.setButtonText("Header");
        addAndMakeVisible(header);

        lowerWindow.setColour(juce::TextButton::buttonColourId, juce::Colours::black);
        addAndMakeVisible(lowerWindow);

        upperWindow.setColour(juce::TextButton::buttonColourId, juce::Colours::black);
        addAndMakeVisible(upperWindow);

        keyboardWindow.setColour(juce::TextButton::buttonColourId, juce::Colours::grey);
        keyboardWindow.setColour(juce::ComboBox::outlineColourId, juce::Colours::white);
        addAndMakeVisible(keyboardWindow);

        divisionInput.setFont(juce::Font(20.0f, juce::Font::bold));
        divisionInput.setText(to_string(divisions), juce::dontSendNotification);
        divisionInput.setColour(juce::Label::textColourId, juce::Colours::white);
        divisionInput.setJustificationType(juce::Justification::centred);
        divisionInput.setEditable(true);
        divisionInput.setColour(juce::Label::backgroundColourId, juce::Colours::maroon);
        divisionInput.setColour(juce::Label::outlineColourId, juce::Colours::white);
        divisionInput.onTextChange = [this] { 

            if (all_of(divisionInput.getText().begin(), divisionInput.getText().end(), isdigit) && divisionInput.getText().getIntValue() >= 12
                && divisionInput.getText().getIntValue() <= 32) {
                divisionInput.setText(divisionInput.getText(), juce::sendNotification);
                divisions = divisionInput.getText().getIntValue();
            }
            else
                divisionInput.setText(to_string(divisions), juce::dontSendNotification);
        };
        addAndMakeVisible(divisionInput);

        divisionLabel.setFont(juce::Font(20.0f, juce::Font::bold));
        divisionLabel.setText("Divisions", juce::dontSendNotification);
        divisionLabel.attachToComponent(&divisionInput, true);
        divisionLabel.setColour(juce::Label::textColourId, juce::Colours::white);
        divisionLabel.setColour(juce::Label::backgroundColourId, juce::Colours::maroon);
        divisionLabel.setColour(juce::Label::outlineColourId, juce::Colours::white);
        divisionLabel.setColour(juce::Label::backgroundColourId, juce::Colours::cornflowerblue);
        divisionLabel.setColour(juce::Label::outlineColourId, juce::Colours::white);
        divisionLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(divisionLabel);

        baseFreqInput.setFont(juce::Font(18.0f, juce::Font::bold));
        baseFreqInput.setText(to_string((int)frequency), juce::dontSendNotification);
        baseFreqInput.setColour(juce::Label::textColourId, juce::Colours::white);
        baseFreqInput.setColour(juce::Label::outlineColourId, juce::Colours::white);
        baseFreqInput.setColour(juce::Label::backgroundColourId, juce::Colours::maroon);
        baseFreqInput.setJustificationType(juce::Justification::centred);
        baseFreqInput.setEditable(true);
        addAndMakeVisible(baseFreqInput);

        baseFreqLabel.setFont(juce::Font(20.0f, juce::Font::bold));
        baseFreqLabel.setText("Frequency", juce::dontSendNotification);
        baseFreqLabel.attachToComponent(&baseFreqInput, true);
        baseFreqLabel.setColour(juce::Label::textColourId, juce::Colours::white);
        baseFreqLabel.setColour(juce::Label::backgroundColourId, juce::Colours::cornflowerblue);
        baseFreqLabel.setColour(juce::Label::outlineColourId, juce::Colours::white);
        baseFreqLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(baseFreqLabel);

        addAndMakeVisible(keyboardComponent);
        setAudioChannels(0, 2);

        footer.setColour(juce::TextButton::buttonColourId, juce::Colours::cornflowerblue);
        footer.setButtonText("Footer");
        addAndMakeVisible(footer);

        sidebar.setColour(juce::TextButton::buttonColourId, juce::Colours::black);
       // sidebar.setButtonText("Sidebar");
        addAndMakeVisible(sidebar);

        

        sideItemA.setColour(juce::TextButton::buttonColourId, juce::Colours::maroon);
        sideItemB.setColour(juce::TextButton::buttonColourId, juce::Colours::maroon);
        sideItemC.setColour(juce::TextButton::buttonColourId, juce::Colours::maroon);
        sideItemA.setButtonText("Item A");
        sideItemB.setButtonText("Item B");
        sideItemC.setButtonText("Item C");
        addAndMakeVisible(sideItemA);
        addAndMakeVisible(sideItemB);
        addAndMakeVisible(sideItemC);

        setSize (1200, 800);
        startTimer (400);
    }

    ~MainContentComponent() override
    {
        shutdownAudio();
    }

    void resized() override
    {
        auto area = getLocalBounds();

        auto headerFooterHeight = 36;
        header.setBounds(area.removeFromTop(headerFooterHeight));
        footer.setBounds(area.removeFromBottom(headerFooterHeight));

        auto sideBarArea = area.removeFromLeft(juce::jmax(80, area.getWidth() / 4));
        sidebar.setBounds(sideBarArea);

        auto sideItemHeight = 40;
        auto sideItemMargin = 5;
        sideItemA.setBounds(sideBarArea.removeFromTop(sideItemHeight).reduced(sideItemMargin));
        sideItemB.setBounds(sideBarArea.removeFromTop(sideItemHeight).reduced(sideItemMargin));
        sideItemC.setBounds(sideBarArea.removeFromTop(sideItemHeight).reduced(sideItemMargin));

        // Set Lower Window
        auto lowerWindowArea = area.removeFromBottom(getHeight() / 3);//.reduced(windowMargin);
        lowerWindow.setBounds(lowerWindowArea);

        // Set Upper Window
        auto upperWindowArea = area.removeFromTop(getHeight() / 3 * 2);//.reduced(windowMargin);
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
        divisionInput.setBounds(keyboardWindow.getX(), keyboardWindow.getY() + keyboardWindow.getHeight() / 2 - divisionHeight, divisionWidth, divisionHeight);//divisionInput.setBounds(keyboardWindow.getX(), keyboardWindow.getY(), divisionWidth, divisionHeight);
        divisionLabel.setBounds(upperWindow.getX(), keyboardWindow.getY() + keyboardWindow.getHeight() / 2 - divisionHeight, upperWindow.getWidth() - keyboardWindowWidth + keyboardWindowMargin, divisionHeight);

        // Set frequency label
        auto frequencyHeight = divisionHeight;
        auto frequencyWidth = divisionWidth;
        baseFreqInput.setBounds(keyboardWindow.getX(), keyboardWindow.getY() + keyboardWindow.getHeight() / 2, frequencyWidth, frequencyHeight);//baseFreqInput.setBounds(keyboardWindow.getX(), keyboardWindow.getY() + keyboardWindow.getHeight() - frequencyHeight, frequencyWidth, frequencyHeight);//baseFreqInput.setBounds(upperWindow.getX() + 200, upperWindow.getY() + 80, frequencyWidth, frequencyHeight);
        baseFreqLabel.setBounds(upperWindow.getX(), keyboardWindow.getY() + keyboardWindow.getHeight() / 2, upperWindow.getWidth() - keyboardWindowWidth + keyboardWindowMargin, frequencyHeight);

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


       /* auto keyboardHeight = 250;
        auto keyboardWidth = 640;
        auto keyboardArea = area.removeFromBottom(keyboardHeight).removeFromLeft(keyboardWidth);
        keyboardComponent.setBounds(keyboardArea);
        keyboardComponent.setBounds(area.removeFromLeft(keyboardWidth));
        keyboardComponent.setKeyWidth(80);
        keyboardComponent.setAvailableRange(72, 84);*/
    }

    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override
    {
        synthAudioSource.prepareToPlay (samplesPerBlockExpected, sampleRate);
    }

    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override
    {
        synthAudioSource.getNextAudioBlock (bufferToFill);
    }

    void releaseResources() override
    {
        synthAudioSource.releaseResources();
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
    juce::TextButton header;
    juce::TextButton sidebar;

    juce::TextButton sideItemA; // [3]
    juce::TextButton sideItemB; // [4]
    juce::TextButton sideItemC; // [5]
    
    juce::TextButton keyboardWindow;
    juce::TextButton lowerWindow;
    juce::TextButton upperWindow;

    juce::Label baseFreqLabel;
    juce::Label divisionLabel;
    juce::Label baseFreqInput;
    juce::Label divisionInput;


    juce::TextButton footer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};
