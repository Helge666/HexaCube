#pragma once
#include <JuceHeader.h>
#include <atomic>
#include "InstrumentConfig.h"
#include "Shared/SampleBank.h"
#include "Shared/VoicePool.h"
#include "Shared/InstrumentParams.h"

class ConfigProcessor : public juce::AudioProcessor
{
public:
    ConfigProcessor();
    ~ConfigProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock&) override {}
    void setStateInformation(const void*, int) override {}

    // ── Config API (UI thread) ────────────────────────────────────────────────
    const InstrumentConfig& getConfig(int instrument) const;
    void setName(int instrument, const juce::String& name);
    void setMidiNote(int instrument, int note);
    void setChokeTarget(int instrument, int chokeTarget);
    void setDisplayOrder(int instrument, int order);
    void requestTrigger(int instrument);   // thread-safe: sets an atomic flag
    void saveConfigToDisk() const;
    juce::String generateCppHeader() const;

private:
    SampleBank sampleBank;
    VoicePool  voicePool;
    bool       samplesLoaded = false;

    InstrumentConfig  configs[16];
    std::atomic<bool> triggerRequests[16];

    static juce::File getConfigFile();
    void loadConfigFromDisk();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ConfigProcessor)
};
