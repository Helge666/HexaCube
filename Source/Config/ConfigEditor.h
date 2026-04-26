#pragma once
#include <JuceHeader.h>
#include "InstrumentPanel.h"

class ConfigProcessor;

class ConfigEditor : public juce::AudioProcessorEditor
{
public:
    explicit ConfigEditor(ConfigProcessor&);
    ~ConfigEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    ConfigProcessor& processor;

    std::unique_ptr<InstrumentPanel> panels[16];

    juce::TextButton saveButton   { "Save Config" };
    juce::TextButton exportButton { "Export C++ Header..." };

    std::unique_ptr<juce::FileChooser> fileChooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ConfigEditor)
};
