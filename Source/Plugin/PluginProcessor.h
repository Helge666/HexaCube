#pragma once
#include <JuceHeader.h>
#include "Shared/SampleBank.h"
#include "Shared/VoicePool.h"
#include "Shared/InstrumentParams.h"

class HexaCubeProcessor : public juce::AudioProcessor
{
public:
    HexaCubeProcessor();
    ~HexaCubeProcessor() override;

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

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

    // MIDI note → instrument index; -1 = unmapped
    // Placeholder chromatic layout (C2–D#3), finalised after iteration 1
    int midiNoteForInstrument[16];

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    SampleBank sampleBank;
    VoicePool  voicePool;
    bool       samplesLoaded = false;

    // Cached raw pointers for audio-thread param reads
    std::atomic<float>* volParams [16] = {};
    std::atomic<float>* panParams [16] = {};
    std::atomic<float>* muteParams[16] = {};

    // Reverse lookup: MIDI note number → instrument index
    int noteToInstrument[128];

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HexaCubeProcessor)
};
