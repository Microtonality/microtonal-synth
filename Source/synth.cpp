/*
  ==============================================================================

    synth.cpp
    Created: 22 Jan 2022 9:37:13pm
    Author:  Michael

  ==============================================================================
*/

#include "synth.h"
#include "Microtonal.h"

namespace IDs
{
    static juce::String paramAttack{ "attack" };
    static juce::String paramDecay{ "decay" };
    static juce::String paramSustain{ "sustain" };
    static juce::String paramRelease{ "release" };
    static juce::String paramGain{ "gain" };
}

//==============================================================================

int Synth::numOscillators = 7;
extern MicrotonalConfig microtonalMappings[7];
extern int mappingGroup;
vector<juce::String> instrumentNames;


void Synth::addADSRParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout)
{
    auto attack = std::make_unique<juce::AudioParameterFloat>(IDs::paramAttack, "Attack", juce::NormalisableRange<float>(0.001f, 0.5f, 0.01f), 0.10f);
    auto decay = std::make_unique<juce::AudioParameterFloat>(IDs::paramDecay, "Decay", juce::NormalisableRange<float>(0.001f, 0.5f, 0.01f), 0.10f);
    auto sustain = std::make_unique<juce::AudioParameterFloat>(IDs::paramSustain, "Sustain", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 1.0f);
    auto release = std::make_unique<juce::AudioParameterFloat>(IDs::paramRelease, "Release", juce::NormalisableRange<float>(0.001f, 0.5f, 0.01f), 0.10f);

    auto group = std::make_unique<juce::AudioProcessorParameterGroup>("adsr", "ADRS", "|",
        std::move(attack),
        std::move(decay),
        std::move(sustain),
        std::move(release));
    layout.add(std::move(group));
}

void Synth::addOvertoneParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout)
{
    auto group = std::make_unique<juce::AudioProcessorParameterGroup>("oscillators", "Oscillators", "|");
    for (int i = 0; i < Synth::numOscillators; ++i)
    {
        group->addChild(std::make_unique<juce::AudioParameterFloat>("osc" + juce::String(i), "Oscillator " + juce::String(i), juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f));
        group->addChild(std::make_unique<juce::AudioParameterFloat>("detune" + juce::String(i), "Detune " + juce::String(i), juce::NormalisableRange<float>(0.0625f, 16.0f, 0.0001f), 1.0f));
		group->addChild(std::make_unique<juce::AudioParameterChoice>("wave_form" + juce::String(i), "wave_form" + juce::String(i),
            juce::StringArray({ "Sin","Squ","Saw","Tri" }),
            0));
	}

    //instrumentNames.push_back("preset1");
    //instrumentNames.push_back("preset2");
    //instrumentNames.push_back("preset3");

    //auto groupInstruments = std::make_unique<juce::AudioProcessorParameterGroup>("instruments", "Instruments", "|");
    //groupInstruments->addChild(std::make_unique<juce::AudioParameterChoice>("instrumentPreset", "Instrument_Preset", juce::StringArray({ "preset342", "preset54"}), 0));


    
    
    //layout.add(std::move(groupInstruments));
    layout.add(std::move(group));

}

void loadInstruments() {    

    try {
        
        juce::File f = juce::File("C:/Users/pdcst/Desktop/microtonal/Configs/instruments.xml");

        if (f.exists())
        {
            DBG("The File is here");
            //juce::XmlDocument doc = juce::XmlDocument(f);
            //juce::XmlDocument dataDoc(f);

            std::unique_ptr<juce::XmlElement> loadedConfig;
            loadedConfig = juce::parseXML(f);

            juce::XmlElement* child = loadedConfig->getFirstChildElement();

            while (child != nullptr)
            {
                    juce::String instrumentName = child->getAttributeValue(0);

                    for (auto* wave : child->getChildIterator())
                    {
                        juce::String shape = wave->getAttributeValue(0);
                        float amp = wave->getAttributeValue(1).getFloatValue();
                        float freq = wave->getAttributeValue(2).getFloatValue();
                    }

                    child = child->getNextElement();
            }

        }
    }
    catch (const std::exception&)
    {
        DBG("No selected instruments");
    }

}



void Synth::addGainParameters(juce::AudioProcessorValueTreeState::ParameterLayout& layout)
{
    auto gain = std::make_unique<juce::AudioParameterFloat>(IDs::paramGain, "Gain", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.70f);

    layout.add(std::make_unique<juce::AudioProcessorParameterGroup>("output", "Output", "|", std::move(gain)));
}

//==============================================================================

Synth::Sound::Sound(juce::AudioProcessorValueTreeState& stateToUse)
    : state(stateToUse)
{
    attack = dynamic_cast<juce::AudioParameterFloat*>(state.getParameter(IDs::paramAttack));
    jassert(attack);
    decay = dynamic_cast<juce::AudioParameterFloat*>(state.getParameter(IDs::paramDecay));
    jassert(decay);
    sustain = dynamic_cast<juce::AudioParameterFloat*>(state.getParameter(IDs::paramSustain));
    jassert(sustain);
    release = dynamic_cast<juce::AudioParameterFloat*>(state.getParameter(IDs::paramRelease));
    jassert(release);
    gain = dynamic_cast<juce::AudioParameterFloat*>(state.getParameter(IDs::paramGain));
    jassert(gain);
}

juce::ADSR::Parameters Synth::Sound::getADSR()
{
    juce::ADSR::Parameters parameters;
    parameters.attack = attack->get();
    parameters.decay = decay->get();
    parameters.sustain = sustain->get();
    parameters.release = release->get();
    return parameters;
}

//==============================================================================

Synth::Voice::Voice(juce::AudioProcessorValueTreeState& state)
{

    for (int i = 0; i < Synth::numOscillators; ++i)
    {
        oscillators.push_back(std::make_unique<BaseOscillator>());
        auto& osc = oscillators.back();
        osc->gain = dynamic_cast<juce::AudioParameterFloat*>(state.getParameter("osc" + juce::String(i)));  
        osc->detune = dynamic_cast<juce::AudioParameterFloat*>(state.getParameter("detune" + juce::String(i)));
        osc->wave_form = dynamic_cast<juce::AudioParameterChoice*>(state.getParameter("wave_form" + juce::String(i)));
        osc->osc.get<0>().initialise([](auto arg) {return std::sin(arg); }, 512);
        osc->multiplier = 1.0;//i + 1;
    }

    gainParameter = dynamic_cast<juce::AudioParameterFloat*>(state.getParameter(IDs::paramGain));
    jassert(gainParameter);

    oscillatorBuffer.setSize(1, internalBufferSize);
    voiceBuffer.setSize(1, internalBufferSize);

    //Comment this out until John figures out the relative pathing part
    loadInstruments();
    
}

bool Synth::Voice::canPlaySound(juce::SynthesiserSound* sound)
{
    return dynamic_cast<Sound*>(sound) != nullptr;
}

void Synth::Voice::startNote(int midiNoteNumber,
    float velocity,
    juce::SynthesiserSound* sound,
    int currentPitchWheelPosition)
{
    juce::ignoreUnused(midiNoteNumber, velocity);

    if (auto* foleysSound = dynamic_cast<Sound*>(sound))
        adsr.setParameters(foleysSound->getADSR());

    pitchWheelValue = getDetuneFromPitchWheel(currentPitchWheelPosition);

    adsr.noteOn();

    for (auto& osc : oscillators)
        updateFrequency(*osc, true);

    //loadInstruments();
}

void Synth::Voice::stopNote(float velocity,
    bool allowTailOff)
{
    juce::ignoreUnused(velocity);

    if (allowTailOff)
    {
        adsr.noteOff();
    }
    else
    {
        adsr.reset();
        clearCurrentNote();
    }
}

void Synth::Voice::pitchWheelMoved(int newPitchWheelValue)
{
    pitchWheelValue = getDetuneFromPitchWheel(newPitchWheelValue);
}

void Synth::Voice::controllerMoved(int controllerNumber, int newControllerValue)
{
    juce::ignoreUnused(controllerNumber, newControllerValue);
}

void Synth::Voice::BaseOscillator::incCurrentAngle() {
    currentAngle += angleDelta;
    if (currentAngle >= juce::MathConstants<float>::twoPi) { currentAngle = fmod(currentAngle, juce::MathConstants<float>::twoPi); }
}

void Synth::Voice::getSamples(BaseOscillator& osc, juce::dsp::ProcessContextReplacing<float>& pc) {
    juce::dsp::AudioBlock<float> buffer = pc.getOutputBlock();
    int totalSamples = buffer.getNumSamples();
    int sampleNum = 0;
    auto oscGain = osc.gain->get();
    auto wave_form = osc.wave_form->getIndex();
    if (oscGain < 0.01)
        return;
    if (wave_form == 0) {
        float sampleSound = 0.0;
        while (sampleNum < totalSamples) {
            sampleSound = std::sin(osc.currentAngle) * oscGain;
            buffer.addSample(0, sampleNum, sampleSound);
            osc.incCurrentAngle();
            sampleNum++;
        }
    }
    else if (wave_form == 1) {
        float sampleSound = 0.0;
        while (sampleNum < totalSamples) {
            sampleSound = osc.currentAngle /
                juce::MathConstants<float>::pi - 1;
            if (sampleSound > (float)0.0) {
                sampleSound = (float)1.0;
            }
            else {
                sampleSound = (float)-1.0;
            }
            sampleSound *= oscGain;
            buffer.addSample(0, sampleNum, sampleSound);
            osc.incCurrentAngle();
            sampleNum++;
        }
    }
    else if (wave_form == 2) {
        float sampleSound = 0.0;
        while (sampleNum < totalSamples) {
            sampleSound = osc.currentAngle /
                juce::MathConstants<float>::pi - 1;
            sampleSound *= oscGain;
            buffer.addSample(0, sampleNum, sampleSound);
            osc.incCurrentAngle();
            sampleNum++;
        }
    }
    else {
        float sampleSound = 0.0;
        while (sampleNum < totalSamples) {
            sampleSound = 2 * osc.currentAngle /
                juce::MathConstants<float>::pi - 2;
            if (sampleSound > 0) {
                sampleSound = sampleSound * -1.0;
            }
            sampleSound += 1;
            sampleSound *= oscGain;
            buffer.addSample(0, sampleNum, sampleSound);
            osc.incCurrentAngle();
            sampleNum++;
        }
    }
   
}

void Synth::Voice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer,
    int startSample,
    int numSamples)
{
    if (!adsr.isActive())
        return;

    while (numSamples > 0)
    {
        auto left = std::min(numSamples, oscillatorBuffer.getNumSamples());
        auto block = juce::dsp::AudioBlock<float>(oscillatorBuffer).getSingleChannelBlock(0).getSubBlock(0, size_t(left));

        juce::dsp::ProcessContextReplacing<float> context(block);
        voiceBuffer.clear();
        for (auto& osc : oscillators)
        {
            oscillatorBuffer.clear();
            getSamples(*osc, context);
            voiceBuffer.addFrom(0, 0, context.getInputBlock().getChannelPointer(0), left);
            /*updateFrequency(*osc);
            osc->osc.get<1>().setGainLinear(oscGain);
            oscillatorBuffer.clear();
            osc->osc.process(context);
            voiceBuffer.addFrom(0, 0, oscillatorBuffer.getReadPointer(0), left);*/
        }

        adsr.applyEnvelopeToBuffer(voiceBuffer, 0, left);

        const auto gain = gainParameter->get();
        outputBuffer.addFromWithRamp(0, startSample, voiceBuffer.getReadPointer(0), left, lastGain, gain);
        lastGain = gain;

        startSample += left;
        numSamples -= left;

        if (!adsr.isActive())
            clearCurrentNote();
    }
}

void Synth::Voice::setCurrentPlaybackSampleRate(double newRate)
{
    juce::SynthesiserVoice::setCurrentPlaybackSampleRate(newRate);

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = newRate;
    spec.maximumBlockSize = juce::uint32(internalBufferSize);
    spec.numChannels = 1;
    for (auto& osc : oscillators)
        osc->osc.prepare(spec);
}

double Synth::Voice::getFrequencyForNote(int noteNumber, double detune, double concertPitch) const
{
    return concertPitch * std::pow(2.0, (noteNumber + detune - 69.0) / 12.0);
}

double Synth::Voice::getDetuneFromPitchWheel(int wheelValue) const
{
    return (wheelValue / 8192.0) - 1.0;
}

void Synth::Voice::updateFrequency(BaseOscillator& oscillator, bool noteStart)
{
    int singleOctaveIndex = (((int)getCurrentlyPlayingNote() - 72) % 12 + 12) % 12, 
        totalSynthIndex = ((int)getCurrentlyPlayingNote() - 72);

    double newFrequency = microtonalMappings[mappingGroup].frequencies[singleOctaveIndex].frequency, 
        defaultFrequency = 440.0 * std::pow(2.0, (float)((int)getCurrentlyPlayingNote() - 81) / 12.0);

    newFrequency = (newFrequency == NULL)
        ? defaultFrequency // note unmapped
        : (totalSynthIndex < 0)
            ? newFrequency * std::pow(2.0, -1.0 * (float)((totalSynthIndex * -1 + 11) / 12)) // note lower than C4
            : newFrequency * std::pow(2.0, (totalSynthIndex / 12)); // note higher than B5

    oscillator.angleDelta = (newFrequency * oscillator.detune->get() / getSampleRate()) * 2.0 * juce::MathConstants<double>::pi;
    if (noteStart) oscillator.currentAngle = 0.0;
    oscillator.osc.get<0>().setFrequency(float(newFrequency * oscillator.detune->get()), noteStart);
}