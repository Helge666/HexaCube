#include "PluginProcessor.h"
#include "PluginEditor.h"

HexaCubeProcessor::HexaCubeProcessor()
    : AudioProcessor(BusesProperties()
        .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{}

HexaCubeProcessor::~HexaCubeProcessor() {}

void HexaCubeProcessor::prepareToPlay(double, int) {}
void HexaCubeProcessor::releaseResources() {}

void HexaCubeProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    buffer.clear();
}

juce::AudioProcessorEditor* HexaCubeProcessor::createEditor()
{
    return new HexaCubeEditor(*this);
}

void HexaCubeProcessor::getStateInformation(juce::MemoryBlock&) {}
void HexaCubeProcessor::setStateInformation(const void*, int) {}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new HexaCubeProcessor();
}
