#pragma once
#include <JuceHeader.h>

class ConfigProcessor;

class InstrumentPanel : public juce::Component
{
public:
    InstrumentPanel(int index, ConfigProcessor& processor);

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    int              index;
    ConfigProcessor& processor;

    juce::Label      indexLabel;
    juce::TextButton triggerButton  { "Trigger" };
    juce::Label      nameLabel      { {}, "Name" };
    juce::TextEditor nameEditor;
    juce::Label      midiLabel      { {}, "MIDI" };
    juce::ComboBox   midiNoteCombo;
    juce::Label      chokeLabel     { {}, "Choke" };
    juce::ComboBox   chokeCombo;
    juce::Label      orderLabel     { {}, "Order" };
    juce::TextButton orderDecButton { "-" };
    juce::Label      orderValueLabel;
    juce::TextButton orderIncButton { "+" };

    void updateOrderValue();
    void populateMidiCombo();
    void populateChokeCombo();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(InstrumentPanel)
};
