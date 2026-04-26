#include "ConfigProcessor.h"
#include "ConfigEditor.h"

ConfigProcessor::ConfigProcessor()
    : AudioProcessor(BusesProperties()
        .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{}

ConfigProcessor::~ConfigProcessor() {}

void ConfigProcessor::prepareToPlay(double, int) {}
void ConfigProcessor::releaseResources() {}

void ConfigProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    buffer.clear();
}

juce::AudioProcessorEditor* ConfigProcessor::createEditor()
{
    return new ConfigEditor(*this);
}

void ConfigProcessor::getStateInformation(juce::MemoryBlock&) {}
void ConfigProcessor::setStateInformation(const void*, int) {}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ConfigProcessor();
}
