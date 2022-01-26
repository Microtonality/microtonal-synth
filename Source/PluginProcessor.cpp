/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//class DivisionComponent : public juce::Component
//{
//public:
//    enum ColourIDs
//    {
//        // we are safe from collissions, because we set the colours on every component directly from the stylesheet
//        backgroundColourId,
//        drawColourId,
//        fillColourId
//    };
//    DivisionComponent()
//    {
//        // make sure you define some default colour, otherwise the juce lookup will choke
//        setColour(backgroundColourId, juce::Colours::black);
//        setColour(drawColourId, juce::Colours::green);
//        setColour(fillColourId, juce::Colours::green.withAlpha(0.5f));
//    }
//    void setFactor(float f)
//    {
//        factor = f;
//    }
//    void paint(juce::Graphics& g) override
//    {
//        g.setColour(findColour(drawColourId));
//        g.fillRect(10, 10, 40, 40);
//    }
//private:
//    float factor = 3.0f;
//    float phase = 0.0f;
//    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DivisionComponent);
//};
//
//class DivisionComponentItem : public foleys::GuiItem
//{
//public:
//    FOLEYS_DECLARE_GUI_FACTORY(DivisionComponentItem)
//    DivisionComponentItem(foleys::MagicGUIBuilder& builder, const juce::ValueTree& node) : foleys::GuiItem(builder, node)
//    {
//        // Create the colour names to have them configurable
//        setColourTranslation({
//            {"divisions-background", DivisionComponent::backgroundColourId},
//            {"divisions-draw", DivisionComponent::drawColourId},
//            {"divisions-fill", DivisionComponent::fillColourId} });
//
//        addAndMakeVisible(mc);
//    }
//    std::vector<foleys::SettableProperty> getSettableProperties() const override
//    {
//        std::vector<foleys::SettableProperty> newProperties;
//
//        newProperties.push_back({ configNode, "factor", foleys::SettableProperty::Number, 1.0f, {} });
//
//        return newProperties;
//    }
//    // Override update() to set the values to your custom component
//    void update() override
//    {
//        auto factor = getProperty("factor");
//        mc.setFactor(factor.isVoid() ? 3.0f : float(factor));
//    }
//
//    juce::Component* getWrappedComponent() override
//    {
//        return &mc;
//    }
//
//private:
//    DivisionComponent mc;
//
//    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DivisionComponentItem)
//};
class DivisionComponent : public juce::Label
{
public:
    enum ColourIDs
    {
        // we are safe from collissions, because we set the colours on every component directly from the stylesheet
        backgroundColourId,
        drawColourId,
        fillColourId
    };
    DivisionComponent()
    {
        // make sure you define some default colour, otherwise the juce lookup will choke
        setColour(backgroundColourId, juce::Colours::black);
        setColour(drawColourId, juce::Colours::green);
        setColour(fillColourId, juce::Colours::green.withAlpha(0.5f));
    }
    void setFactor(float f)
    {
        factor = f;
    }
    void paint(juce::Graphics& g) override
    {
        g.setColour(findColour(drawColourId));
       
        g.fillRect(10, 10, 40, 40);
        
        
    }
    //void resized() override
    //{
    //    auto area = getLocalBounds();
    //    division.setBounds(area.removeFromBottom(20));

    //}
private:
    float factor = 3.0f;
    float phase = 0.0f;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DivisionComponent);
};

class DivisionComponentItem : public foleys::GuiItem
{
public:
    FOLEYS_DECLARE_GUI_FACTORY(DivisionComponentItem)
    DivisionComponentItem(foleys::MagicGUIBuilder& builder, const juce::ValueTree& node) : foleys::GuiItem(builder, node)
    {
        // Create the colour names to have them configurable
        setColourTranslation({
            {"divisions-background", DivisionComponent::backgroundColourId},
            {"divisions-draw", DivisionComponent::drawColourId},
            {"divisions-fill", DivisionComponent::fillColourId} });

        addAndMakeVisible(mc);
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
        mc.setFactor(factor.isVoid() ? 3.0f : float(factor));
    }

    juce::Component* getWrappedComponent() override
    {
        return &mc;
    }

private:
    DivisionComponent mc;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DivisionComponentItem)
};
//==============================================================================
MicrotonalSynthAudioProcessor::MicrotonalSynthAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : MagicProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    FOLEYS_SET_SOURCE_PATH(__FILE__);
    magicState.setGuiValueTree(BinaryData::ProposedLayout_xml, BinaryData::ProposedLayout_xmlSize);
    divisions = 12;
}

MicrotonalSynthAudioProcessor::~MicrotonalSynthAudioProcessor()
{
}

//==============================================================================
const juce::String MicrotonalSynthAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MicrotonalSynthAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool MicrotonalSynthAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool MicrotonalSynthAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double MicrotonalSynthAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MicrotonalSynthAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int MicrotonalSynthAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MicrotonalSynthAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String MicrotonalSynthAudioProcessor::getProgramName (int index)
{
    return {};
}

void MicrotonalSynthAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//bool MicrotonalSynthAudioProcessor::hasEditor() const
//{
//    return true;
//}

//==============================================================================
void MicrotonalSynthAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void MicrotonalSynthAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

void MicrotonalSynthAudioProcessor::initialiseBuilder(foleys::MagicGUIBuilder& builder)
{
    builder.registerJUCEFactories();

    builder.registerFactory("Divisions", &DivisionComponentItem::factory);
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MicrotonalSynthAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void MicrotonalSynthAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        // ..do something to the data...
    }
}

//==============================================================================

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MicrotonalSynthAudioProcessor();
}
