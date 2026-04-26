#include "Voice.h"

void Voice::trigger(const juce::AudioBuffer<float>* buf, int inst, float velGain,
                    double advance, juce::uint32 time)
{
    buffer          = buf;
    instrument      = inst;
    position        = 0.0;
    positionAdvance = advance;
    velocityGain    = velGain;
    active          = true;
    startTime       = time;
}

void Voice::renderNextBlock(juce::AudioBuffer<float>& output, int startSample, int numSamples,
                             float totalGain, float pan)
{
    if (!active || buffer == nullptr)
        return;

    const int srcLength   = buffer->getNumSamples();
    const int srcChannels = buffer->getNumChannels();

    // Equal-power panning: pan in [-1, 1] → angle in [0, pi/2]
    const float angle = (pan + 1.0f) * juce::MathConstants<float>::pi * 0.25f;
    const float gainL = totalGain * std::cos(angle);
    const float gainR = totalGain * std::sin(angle);

    float* outL = output.getWritePointer(0, startSample);
    float* outR = output.getNumChannels() > 1 ? output.getWritePointer(1, startSample) : nullptr;

    for (int i = 0; i < numSamples; ++i)
    {
        if (position >= (double)(srcLength - 1))
        {
            active = false;
            break;
        }

        const int   pos0 = (int)position;
        const float frac = (float)(position - pos0);
        const int   pos1 = pos0 + 1;

        // Linear interpolation; mono source uses ch0 for both channels
        const float srcL = buffer->getSample(0, pos0) + frac * (buffer->getSample(0, pos1) - buffer->getSample(0, pos0));
        const float srcR = (srcChannels > 1)
                         ? buffer->getSample(1, pos0) + frac * (buffer->getSample(1, pos1) - buffer->getSample(1, pos0))
                         : srcL;

        outL[i] += srcL * gainL;
        if (outR != nullptr)
            outR[i] += srcR * gainR;

        position += positionAdvance;
    }
}
