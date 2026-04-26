#include "PluginProcessor.h"
#include "PluginEditor.h"

HexaCubeProcessor::HexaCubeProcessor()
    : AudioProcessor(BusesProperties()
          .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "HexaCubeState", createParameterLayout())
{
    // Cache APVTS raw parameter pointers for lock-free audio-thread access
    for (int i = 0; i < 16; ++i)
    {
        volParams [i] = apvts.getRawParameterValue("vol_"  + juce::String(i));
        panParams [i] = apvts.getRawParameterValue("pan_"  + juce::String(i));
        muteParams[i] = apvts.getRawParameterValue("mute_" + juce::String(i));
    }

    // Placeholder MIDI note map: chromatic from C2 (36). Finalised after iteration 1.
    for (int i = 0; i < 16; ++i)
        midiNoteForInstrument[i] = 36 + i;

    std::fill(std::begin(noteToInstrument), std::end(noteToInstrument), -1);
    for (int i = 0; i < 16; ++i)
        noteToInstrument[midiNoteForInstrument[i]] = i;
}

HexaCubeProcessor::~HexaCubeProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout HexaCubeProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    for (int i = 0; i < 16; ++i)
    {
        const auto id   = juce::String(i);
        const auto name = "Instrument " + id;

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{ "vol_"  + id, 1 }, name + " Volume",
            juce::NormalisableRange<float>(0.0f, 1.0f), 0.8f));

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{ "pan_"  + id, 1 }, name + " Pan",
            juce::NormalisableRange<float>(-1.0f, 1.0f), 0.0f));

        layout.add(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID{ "mute_" + id, 1 }, name + " Mute", false));
    }

    return layout;
}

void HexaCubeProcessor::prepareToPlay(double sampleRate, int)
{
    if (!samplesLoaded)
    {
        sampleBank.load();
        samplesLoaded = true;
    }
    voicePool.prepare(sampleRate, sampleBank);
}

void HexaCubeProcessor::releaseResources() {}

void HexaCubeProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    buffer.clear();

    for (const auto metadata : midiMessages)
    {
        const auto msg = metadata.getMessage();
        if (msg.isNoteOn() && msg.getVelocity() > 0)
        {
            const int note = msg.getNoteNumber();
            if (note >= 0 && note < 128)
            {
                const int instrument = noteToInstrument[note];
                if (instrument >= 0)
                    voicePool.noteOn(instrument, msg.getVelocity());
            }
        }
    }

    InstrumentParams params[16];
    for (int i = 0; i < 16; ++i)
    {
        params[i].volume = volParams [i]->load();
        params[i].pan    = panParams [i]->load();
        params[i].mute   = muteParams[i]->load() > 0.5f;
    }

    voicePool.renderNextBlock(buffer, 0, buffer.getNumSamples(), params);
}

juce::AudioProcessorEditor* HexaCubeProcessor::createEditor()
{
    return new HexaCubeEditor(*this);
}

void HexaCubeProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    auto xml   = std::make_unique<juce::XmlElement>("HexaCubeState");
    xml->addChildElement(state.createXml().release());

    // Persist MIDI note assignments
    auto* noteMap = xml->createNewChildElement("MidiMap");
    for (int i = 0; i < 16; ++i)
        noteMap->setAttribute("inst" + juce::String(i), midiNoteForInstrument[i]);

    copyXmlToBinary(*xml, destData);
}

void HexaCubeProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    auto xml = getXmlFromBinary(data, sizeInBytes);
    if (xml == nullptr || !xml->hasTagName("HexaCubeState"))
        return;

    if (auto* apvtsXml = xml->getFirstChildElement())
        apvts.replaceState(juce::ValueTree::fromXml(*apvtsXml));

    if (auto* noteMap = xml->getChildByName("MidiMap"))
    {
        std::fill(std::begin(noteToInstrument), std::end(noteToInstrument), -1);
        for (int i = 0; i < 16; ++i)
        {
            midiNoteForInstrument[i] = noteMap->getIntAttribute("inst" + juce::String(i),
                                                                 36 + i);
            noteToInstrument[midiNoteForInstrument[i]] = i;
        }
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new HexaCubeProcessor();
}
