#pragma once
#include <JuceHeader.h>
#include "ConfigProcessor.h"

class ConfigEditor : public juce::AudioProcessorEditor
{
public:
    explicit ConfigEditor(ConfigProcessor&);
    ~ConfigEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    ConfigProcessor& processor;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ConfigEditor)
};
