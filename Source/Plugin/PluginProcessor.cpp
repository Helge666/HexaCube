#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Shared/InstrumentData.h"

HexaCubeProcessor::HexaCubeProcessor()
    : AudioProcessor(BusesProperties()
          .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "HexaCubeState", createParameterLayout())
{
    for (int i = 0; i < 16; ++i)
    {
        volParams [i] = apvts.getRawParameterValue("vol_"  + juce::String(i));
        panParams [i] = apvts.getRawParameterValue("pan_"  + juce::String(i));
        muteParams[i] = apvts.getRawParameterValue("mute_" + juce::String(i));
        midiNoteForInstrument[i].store(DEFAULT_MIDI_NOTES[i]);
        triggerRequests[i].store(false);
    }
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
            juce::ParameterID{ "vol_" + id, 1 }, name + " Volume",
            juce::NormalisableRange<float>(0.0f, 1.0f), 0.8f));

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{ "pan_" + id, 1 }, name + " Pan",
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

    // UI trigger requests from message thread
    for (int i = 0; i < 16; ++i)
    {
        if (triggerRequests[i].exchange(false))
        {
            for (int j = 0; CHOKE_PAIRS[j][0] >= 0; ++j)
                if (CHOKE_PAIRS[j][0] == i)
                    voicePool.chokeInstrument(CHOKE_PAIRS[j][1]);
            voicePool.noteOn(i, 100);
        }
    }

    // Hardware / DAW MIDI input
    for (const auto& meta : midiMessages)
    {
        const auto msg = meta.getMessage();
        if (msg.isNoteOn() && msg.getVelocity() > 0)
        {
            const int note = msg.getNoteNumber();
            for (int i = 0; i < 16; ++i)
            {
                if (midiNoteForInstrument[i].load() == note)
                {
                    for (int j = 0; CHOKE_PAIRS[j][0] >= 0; ++j)
                        if (CHOKE_PAIRS[j][0] == i)
                            voicePool.chokeInstrument(CHOKE_PAIRS[j][1]);
                    voicePool.noteOn(i, msg.getVelocity());
                    break;
                }
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

void HexaCubeProcessor::setMidiNote(int instrument, int note)
{
    jassert(instrument >= 0 && instrument < 16);
    jassert(note >= 0 && note < 128);
    midiNoteForInstrument[instrument].store(note);
}

void HexaCubeProcessor::requestTrigger(int instrument)
{
    jassert(instrument >= 0 && instrument < 16);
    triggerRequests[instrument].store(true);
}

void HexaCubeProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    auto xml   = std::make_unique<juce::XmlElement>("HexaCubeState");
    xml->addChildElement(state.createXml().release());

    auto* noteMap = xml->createNewChildElement("MidiMap");
    for (int i = 0; i < 16; ++i)
        noteMap->setAttribute("inst" + juce::String(i), midiNoteForInstrument[i].load());

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
        for (int i = 0; i < 16; ++i)
            midiNoteForInstrument[i].store(
                noteMap->getIntAttribute("inst" + juce::String(i), DEFAULT_MIDI_NOTES[i]));
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new HexaCubeProcessor();
}
