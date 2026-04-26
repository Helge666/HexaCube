#include "ConfigProcessor.h"
#include "ConfigEditor.h"
#include <algorithm>

ConfigProcessor::ConfigProcessor()
    : AudioProcessor(BusesProperties()
          .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
    for (auto& r : triggerRequests)
        r.store(false);

    // Default MIDI note map: chromatic from C2 (36). Finalised after iteration 1.
    for (int i = 0; i < 16; ++i)
    {
        configs[i].midiNote     = 36 + i;
        configs[i].displayOrder = i + 1;
    }

    loadConfigFromDisk();
}

ConfigProcessor::~ConfigProcessor() {}

// ── AudioProcessor ────────────────────────────────────────────────────────────

void ConfigProcessor::prepareToPlay(double sampleRate, int)
{
    if (!samplesLoaded)
    {
        sampleBank.load();
        samplesLoaded = true;
    }
    voicePool.prepare(sampleRate, sampleBank);
}

void ConfigProcessor::releaseResources() {}

void ConfigProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    buffer.clear();

    // UI trigger requests — atomic flags set from message thread
    for (int i = 0; i < 16; ++i)
    {
        if (triggerRequests[i].exchange(false))
        {
            // NOTE: configs[] is read here on the audio thread while the UI may write it.
            // Acceptable for a dev tool — worst case: missed or stale choke target.
            const int target = configs[i].chokeTarget;
            if (target >= 0)
                voicePool.chokeInstrument(target);
            voicePool.noteOn(i, 100);
        }
    }

    // Hardware MIDI input from the standalone wrapper
    for (const auto& metadata : midiMessages)
    {
        const auto msg = metadata.getMessage();
        if (msg.isNoteOn() && msg.getVelocity() > 0)
        {
            const int note = msg.getNoteNumber();
            for (int i = 0; i < 16; ++i)
            {
                if (configs[i].midiNote == note)
                {
                    const int target = configs[i].chokeTarget;
                    if (target >= 0)
                        voicePool.chokeInstrument(target);
                    voicePool.noteOn(i, msg.getVelocity());
                    break;
                }
            }
        }
    }

    InstrumentParams params[16]; // all defaults: vol 1, pan 0, mute false
    voicePool.renderNextBlock(buffer, 0, buffer.getNumSamples(), params);
}

juce::AudioProcessorEditor* ConfigProcessor::createEditor()
{
    return new ConfigEditor(*this);
}

// ── Config API ────────────────────────────────────────────────────────────────

const InstrumentConfig& ConfigProcessor::getConfig(int i) const
{
    jassert(i >= 0 && i < 16);
    return configs[i];
}

void ConfigProcessor::setName(int i, const juce::String& name)
{
    configs[i].name = name;
}

void ConfigProcessor::setMidiNote(int i, int note)
{
    configs[i].midiNote = note;
}

void ConfigProcessor::setChokeTarget(int i, int target)
{
    configs[i].chokeTarget = target;
}

void ConfigProcessor::setDisplayOrder(int i, int order)
{
    configs[i].displayOrder = juce::jlimit(1, 16, order);
}

void ConfigProcessor::requestTrigger(int i)
{
    jassert(i >= 0 && i < 16);
    triggerRequests[i].store(true);
}

// ── Persistence ───────────────────────────────────────────────────────────────

juce::File ConfigProcessor::getConfigFile()
{
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
               .getChildFile("voXager/HexaCube/config.xml");
}

void ConfigProcessor::loadConfigFromDisk()
{
    const auto file = getConfigFile();
    if (!file.existsAsFile())
        return;

    const auto xml = juce::XmlDocument::parse(file);
    if (xml == nullptr || !xml->hasTagName("HexaCubeConfig"))
        return;

    for (auto* el = xml->getFirstChildElement(); el != nullptr; el = el->getNextElement())
    {
        if (!el->hasTagName("Instrument"))
            continue;
        const int i = el->getIntAttribute("index", -1);
        if (i < 0 || i >= 16)
            continue;

        configs[i].name         = el->getStringAttribute("name",         configs[i].name);
        configs[i].midiNote     = el->getIntAttribute   ("midiNote",     configs[i].midiNote);
        configs[i].chokeTarget  = el->getIntAttribute   ("chokeTarget",  -1);
        configs[i].displayOrder = el->getIntAttribute   ("displayOrder", i + 1);
    }
}

void ConfigProcessor::saveConfigToDisk() const
{
    const auto file = getConfigFile();
    file.getParentDirectory().createDirectory();

    juce::XmlElement xml("HexaCubeConfig");
    for (int i = 0; i < 16; ++i)
    {
        auto* el = xml.createNewChildElement("Instrument");
        el->setAttribute("index",        i);
        el->setAttribute("name",         configs[i].name);
        el->setAttribute("midiNote",     configs[i].midiNote);
        el->setAttribute("chokeTarget",  configs[i].chokeTarget);
        el->setAttribute("displayOrder", configs[i].displayOrder);
    }
    xml.writeTo(file);
}

// ── C++ header export ─────────────────────────────────────────────────────────

juce::String ConfigProcessor::generateCppHeader() const
{
    // Sort instruments by display order for the ordered array
    int order[16];
    for (int i = 0; i < 16; ++i) order[i] = i;
    std::sort(order, order + 16, [this](int a, int b)
    {
        return configs[a].displayOrder < configs[b].displayOrder;
    });

    juce::String s;
    s << "// ================================================================\n";
    s << "// HexaCube Instrument Configuration\n";
    s << "// Generated by HexaCube Config Tool — copy to Source/Shared/InstrumentData.h\n";
    s << "// ================================================================\n\n";
    s << "#pragma once\n\n";

    s << "static const char* const INSTRUMENT_NAMES[16] =\n{\n";
    for (int i = 0; i < 16; ++i)
        s << "    /* " << juce::String::formatted("%02d", i) << " */ \""
          << configs[i].name << "\",\n";
    s << "};\n\n";

    s << "// Instrument indices in display order\n";
    s << "static const int DISPLAY_ORDER[16] = {";
    for (int i = 0; i < 16; ++i)
        s << " " << order[i] << (i < 15 ? "," : "");
    s << " };\n\n";

    s << "// Default MIDI note assignments\n";
    s << "static const int DEFAULT_MIDI_NOTES[16] =\n{\n";
    for (int i = 0; i < 16; ++i)
        s << "    /* " << juce::String::formatted("%02d", i)
          << " \"" << configs[i].name << "\" */ " << configs[i].midiNote << ",\n";
    s << "};\n\n";

    s << "// Cross-choke pairs: { source_instrument, choked_instrument }\n";
    s << "// (source chokes target when triggered; self-choke is always implicit)\n";
    s << "static const int CHOKE_PAIRS[][2] =\n{\n";
    bool any = false;
    for (int i = 0; i < 16; ++i)
    {
        if (configs[i].chokeTarget >= 0)
        {
            s << "    { " << i << ", " << configs[i].chokeTarget << " },  // \""
              << configs[i].name << "\" chokes \""
              << configs[configs[i].chokeTarget].name << "\"\n";
            any = true;
        }
    }
    if (!any)
        s << "    // No cross-choke pairs defined\n";
    s << "    { -1, -1 }  // sentinel\n";
    s << "};\n";

    return s;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ConfigProcessor();
}
