#pragma once
#include <JuceHeader.h>

struct Voice
{
    const juce::AudioBuffer<float>* buffer = nullptr;
    int           instrument       = -1;
    double        position         = 0.0;
    double        positionAdvance  = 1.0;
    float         velocityGain     = 1.0f;
    bool          active           = false;
    juce::uint32  startTime        = 0;

    void trigger(const juce::AudioBuffer<float>* buf, int inst, float velGain,
                 double advance, juce::uint32 time);

    void choke() { active = false; }

    // totalGain = velocityGain * instrumentVolume; pan in [-1, 1]
    void renderNextBlock(juce::AudioBuffer<float>& output, int startSample, int numSamples,
                         float totalGain, float pan);
};
