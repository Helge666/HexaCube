#include "SampleBank.h"
#include <BinaryData.h>

void SampleBank::load()
{
    juce::WavAudioFormat wavFormat;

    for (int inst = 0; inst < 16; ++inst)
    {
        for (int vel = 0; vel < 6; ++vel)
        {
            for (int rr = 0; rr < 4; ++rr)
            {
                char name[32];
                std::snprintf(name, sizeof(name), "snd%02dvel%02drr%02d_wav",
                              inst + 1, vel + 1, rr + 1);

                int dataSize = 0;
                const char* data = BinaryData::getNamedResource(name, dataSize);
                jassert(data != nullptr);
                if (data == nullptr)
                    continue;

                auto stream = std::make_unique<juce::MemoryInputStream>(data, (size_t)dataSize, false);
                std::unique_ptr<juce::AudioFormatReader> reader(
                    wavFormat.createReaderFor(stream.release(), true));

                jassert(reader != nullptr);
                if (reader == nullptr)
                    continue;

                sourceSampleRate = reader->sampleRate;

                auto& buf = samples[inst][vel][rr];
                buf.setSize((int)reader->numChannels, (int)reader->lengthInSamples);
                reader->read(&buf, 0, (int)reader->lengthInSamples, 0, true, true);
            }
        }
    }
}

const juce::AudioBuffer<float>& SampleBank::getSample(int instrument, int velocityLayer, int roundRobin) const
{
    jassert(instrument   >= 0 && instrument   < 16);
    jassert(velocityLayer >= 0 && velocityLayer < 6);
    jassert(roundRobin   >= 0 && roundRobin   < 4);
    return samples[instrument][velocityLayer][roundRobin];
}
