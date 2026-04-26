#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class InstrumentStrip : public juce::Component
{
public:
    InstrumentStrip(HexaCubeProcessor& p, int instrumentIndex);
    ~InstrumentStrip() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    HexaCubeProcessor& processor;
    int index;

    juce::Label        nameLabel;
    juce::TextButton   triggerButton { ">" };
    juce::ComboBox     midiCombo;
    juce::Label        volLabel;
    juce::Slider       volSlider;
    juce::Label        panLabel;
    juce::Slider       panSlider;
    juce::ToggleButton muteButton    { "Mute" };

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> volAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> panAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> muteAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(InstrumentStrip)
};
