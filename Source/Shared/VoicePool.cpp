#include "VoicePool.h"
#include <limits>

void VoicePool::prepare(double hostSampleRate, const SampleBank& bank)
{
    sampleBank    = &bank;
    positionAdvance = bank.getSourceSampleRate() / hostSampleRate;
    sampleTime    = 0;
    std::fill(std::begin(rrCounters), std::end(rrCounters), 0);

    for (auto& v : voices)
        v.choke();
}

void VoicePool::noteOn(int instrument, int midiVelocity)
{
    jassert(instrument >= 0 && instrument < 16);
    jassert(sampleBank != nullptr);

    chokeInstrument(instrument);   // self-choke: hard-cut any previous voices

    const int layer = velocityToLayer(midiVelocity);
    const int rr    = rrCounters[instrument] % 4;
    ++rrCounters[instrument];

    const float velGain = midiVelocity / 127.0f;
    const int   vi      = findFreeVoice(instrument);

    voices[vi].trigger(&sampleBank->getSample(instrument, layer, rr),
                       instrument, velGain, positionAdvance, sampleTime);
}

void VoicePool::chokeInstrument(int instrument)
{
    for (auto& v : voices)
        if (v.active && v.instrument == instrument)
            v.choke();
}

void VoicePool::renderNextBlock(juce::AudioBuffer<float>& output, int startSample, int numSamples,
                                 const InstrumentParams params[16])
{
    for (auto& v : voices)
    {
        if (!v.active)
            continue;

        const auto& p = params[v.instrument];
        if (p.mute)
            continue;

        v.renderNextBlock(output, startSample, numSamples,
                          v.velocityGain * p.volume, p.pan);
    }

    sampleTime += (juce::uint32)numSamples;
}

int VoicePool::findFreeVoice(int instrument) const
{
    // Prefer a genuinely inactive voice
    for (int i = 0; i < NumVoices; ++i)
        if (!voices[i].active)
            return i;

    // Steal oldest active voice of the same instrument
    int oldest = -1;
    juce::uint32 oldestTime = std::numeric_limits<juce::uint32>::max();
    for (int i = 0; i < NumVoices; ++i)
    {
        if (voices[i].instrument == instrument && voices[i].startTime < oldestTime)
        {
            oldest = i;
            oldestTime = voices[i].startTime;
        }
    }
    if (oldest >= 0)
        return oldest;

    // Global fallback: steal globally oldest voice
    oldest = 0;
    oldestTime = voices[0].startTime;
    for (int i = 1; i < NumVoices; ++i)
    {
        if (voices[i].startTime < oldestTime)
        {
            oldest = i;
            oldestTime = voices[i].startTime;
        }
    }
    return oldest;
}

int VoicePool::velocityToLayer(int midiVelocity)
{
    return std::min((midiVelocity - 1) / 21, 5);
}
