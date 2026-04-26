#pragma once
#include <JuceHeader.h>
#include "Voice.h"
#include "SampleBank.h"
#include "InstrumentParams.h"

class VoicePool
{
public:
    static constexpr int NumVoices = 32;

    void prepare(double hostSampleRate, const SampleBank& bank);
    void noteOn(int instrument, int midiVelocity);
    void chokeInstrument(int instrument);
    void renderNextBlock(juce::AudioBuffer<float>& output, int startSample, int numSamples,
                         const InstrumentParams params[16]);

private:
    Voice             voices[NumVoices];
    const SampleBank* sampleBank     = nullptr;
    double            positionAdvance = 1.0;
    int               rrCounters[16] = {};
    juce::uint32      sampleTime     = 0;

    int  findFreeVoice(int instrument) const;
    static int velocityToLayer(int midiVelocity);
};
