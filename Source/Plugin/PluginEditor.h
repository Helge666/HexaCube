#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class HexaCubeEditor : public juce::AudioProcessorEditor
{
public:
    explicit HexaCubeEditor(HexaCubeProcessor&);
    ~HexaCubeEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    HexaCubeProcessor& processor;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HexaCubeEditor)
};
