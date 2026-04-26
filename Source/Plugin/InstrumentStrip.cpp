#include "InstrumentStrip.h"
#include "Shared/InstrumentData.h"

InstrumentStrip::InstrumentStrip(HexaCubeProcessor& p, int i)
    : processor(p), index(i)
{
    nameLabel.setText(INSTRUMENT_NAMES[i], juce::dontSendNotification);
    nameLabel.setFont(juce::Font(11.5f, juce::Font::bold));
    nameLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(nameLabel);

    triggerButton.onClick = [this] { processor.requestTrigger(index); };
    addAndMakeVisible(triggerButton);

    for (int n = 0; n < 128; ++n)
    {
        const auto name = juce::MidiMessage::getMidiNoteName(n, true, true, 3)
                          + " (" + juce::String(n) + ")";
        midiCombo.addItem(name, n + 1);
    }
    midiCombo.setSelectedId(processor.midiNoteForInstrument[i].load() + 1,
                            juce::dontSendNotification);
    midiCombo.onChange = [this]
    {
        processor.setMidiNote(index, midiCombo.getSelectedId() - 1);
    };
    addAndMakeVisible(midiCombo);

    volLabel.setText("Vol", juce::dontSendNotification);
    volLabel.setFont(juce::Font(10.0f));
    volLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(volLabel);

    volSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    volSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(volSlider);
    volAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.apvts, "vol_" + juce::String(i), volSlider);

    panLabel.setText("Pan", juce::dontSendNotification);
    panLabel.setFont(juce::Font(10.0f));
    panLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(panLabel);

    panSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    panSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(panSlider);
    panAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.apvts, "pan_" + juce::String(i), panSlider);

    muteAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        processor.apvts, "mute_" + juce::String(i), muteButton);
    addAndMakeVisible(muteButton);
}

InstrumentStrip::~InstrumentStrip() {}

void InstrumentStrip::paint(juce::Graphics& g)
{
    g.setColour(juce::Colour(0xff3a3a3a));
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 4.0f);
    g.setColour(juce::Colour(0xff555555));
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(0.5f), 4.0f, 1.0f);
}

void InstrumentStrip::resized()
{
    auto area = getLocalBounds().reduced(5);

    // Row 1: instrument name + trigger button
    auto row = area.removeFromTop(22);
    triggerButton.setBounds(row.removeFromRight(30));
    row.removeFromRight(3);
    nameLabel.setBounds(row);

    area.removeFromTop(3);

    // Row 2: MIDI note combo
    midiCombo.setBounds(area.removeFromTop(20));

    area.removeFromTop(3);

    // Row 3: volume
    row = area.removeFromTop(22);
    volLabel.setBounds(row.removeFromLeft(28));
    volSlider.setBounds(row);

    area.removeFromTop(2);

    // Row 4: pan
    row = area.removeFromTop(22);
    panLabel.setBounds(row.removeFromLeft(28));
    panSlider.setBounds(row);

    area.removeFromTop(3);

    // Row 5: mute
    muteButton.setBounds(area.removeFromTop(22).removeFromLeft(55));
}
