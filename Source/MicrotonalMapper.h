/*
  ==============================================================================

    MicrotonalMapper.h
    Created: 9 Feb 2022 11:10:24am
    Author:  Michael

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include <algorithm>
#include <string> 
#include <cctype> 
#include "Microtonal.h"
using namespace std;

//==============================================================================
extern MicrotonalConfig microtonalMappings[7];
extern int mappingIndex;
//=================================================================================================
struct SineWaveSound : public juce::SynthesiserSound
{
    SineWaveSound() {}

    bool appliesToNote(int) override { return true; }
    bool appliesToChannel(int) override { return true; }
};
//=================================================================================================
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
        if (microtonalMappings[mappingIndex].frequencies[midiNoteNumber - 72].frequency == NULL) cyclesPerSecond = 440.0 * std::pow(2.0, (midiNoteNumber - 69) / 12.0); //change this for key mapping
        else  cyclesPerSecond = microtonalMappings[mappingIndex].frequencies[midiNoteNumber - 72].frequency; //change this for key mapping

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
//=================================================================================================
class SynthAudioSource : public juce::AudioSource
{
public:
    SynthAudioSource(juce::MidiKeyboardState& keyState);

    void setUsingSineWaveSound();

    void prepareToPlay(int /*samplesPerBlockExpected*/, double sampleRate) override;

    void releaseResources() override;

    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;

private:
    juce::MidiKeyboardState& keyboardState;
    juce::Synthesiser synth;
};
//=================================================================================================
class MainContentComponent : public juce::AudioAppComponent,
    private juce::Timer,
    public juce::Button::Listener
{
public:
    MainContentComponent(int index);

    ~MainContentComponent() override;

    void resized() override;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;

    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill);

    void releaseResources();

    void buttonClicked(juce::Button* btn) override;

    void undoButtonHighlighting();

    void genFreqFunc();

    string writeValuesToXML();

    void loadConfig();

    float roundoff(float value, unsigned char prec);

    void paint(juce::Graphics& g) override;

private:
    juce::Array<juce::Colour> colours { 
        juce::Colour(49,49,49),   // background color
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

    void timerCallback() override;
    int test = 1;
    int divisions = 12;
    double frequency = 440.0;
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
    juce::TextButton saveToXMLBtn;

    int startKey = 72;
    juce::TextButton frequencyBoxes[24];
    juce::TextButton noteButtons[12];

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainContentComponent)
};
//=================================================================================================
