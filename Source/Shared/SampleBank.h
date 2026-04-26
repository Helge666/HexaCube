#pragma once
#include <JuceHeader.h>

class SampleBank
{
public:
    void load();

    // All indices are 0-based: instrument 0–15, velocityLayer 0–5, roundRobin 0–3
    const juce::AudioBuffer<float>& getSample(int instrument, int velocityLayer, int roundRobin) const;

    double getSourceSampleRate() const { return sourceSampleRate; }

private:
    juce::AudioBuffer<float> samples[16][6][4];
    double sourceSampleRate = 44100.0;
};
