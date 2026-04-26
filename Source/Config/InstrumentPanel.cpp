#include "InstrumentPanel.h"
#include "ConfigProcessor.h"

InstrumentPanel::InstrumentPanel(int idx, ConfigProcessor& proc)
    : index(idx), processor(proc)
{
    const auto& cfg = processor.getConfig(index);

    // ── Index label ──────────────────────────────────────────────────────────
    indexLabel.setText(juce::String::formatted("%02d", index + 1),
                       juce::dontSendNotification);
    indexLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    indexLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(indexLabel);

    // ── Trigger button ───────────────────────────────────────────────────────
    triggerButton.setColour(juce::TextButton::buttonColourId,
                            juce::Colour(0xff2a6e2a));
    triggerButton.setColour(juce::TextButton::buttonOnColourId,
                            juce::Colour(0xff44aa44));
    triggerButton.onClick = [this] { processor.requestTrigger(index); };
    addAndMakeVisible(triggerButton);

    // ── Name ─────────────────────────────────────────────────────────────────
    nameLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    nameLabel.setFont(juce::Font(11.0f));
    addAndMakeVisible(nameLabel);

    nameEditor.setText(cfg.name, juce::dontSendNotification);
    nameEditor.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff1e1e1e));
    nameEditor.setColour(juce::TextEditor::textColourId, juce::Colours::white);
    nameEditor.onFocusLost = [this]
    {
        processor.setName(index, nameEditor.getText());
        processor.saveConfigToDisk();
    };
    nameEditor.onReturnKey = [this]
    {
        processor.setName(index, nameEditor.getText());
        processor.saveConfigToDisk();
    };
    addAndMakeVisible(nameEditor);

    // ── MIDI note ─────────────────────────────────────────────────────────────
    midiLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    midiLabel.setFont(juce::Font(11.0f));
    addAndMakeVisible(midiLabel);

    populateMidiCombo();
    midiNoteCombo.setSelectedId(cfg.midiNote + 1, juce::dontSendNotification);
    midiNoteCombo.onChange = [this]
    {
        processor.setMidiNote(index, midiNoteCombo.getSelectedId() - 1);
        processor.saveConfigToDisk();
    };
    addAndMakeVisible(midiNoteCombo);

    // ── Choke target ──────────────────────────────────────────────────────────
    chokeLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    chokeLabel.setFont(juce::Font(11.0f));
    addAndMakeVisible(chokeLabel);

    populateChokeCombo();
    const int chokeId = (cfg.chokeTarget < 0) ? 1 : (cfg.chokeTarget + 2);
    chokeCombo.setSelectedId(chokeId, juce::dontSendNotification);
    chokeCombo.onChange = [this]
    {
        const int id = chokeCombo.getSelectedId();
        processor.setChokeTarget(index, (id == 1) ? -1 : (id - 2));
        processor.saveConfigToDisk();
    };
    addAndMakeVisible(chokeCombo);

    // ── Display order ─────────────────────────────────────────────────────────
    orderLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    orderLabel.setFont(juce::Font(11.0f));
    addAndMakeVisible(orderLabel);

    orderValueLabel.setJustificationType(juce::Justification::centred);
    orderValueLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    orderValueLabel.setFont(juce::Font(12.0f));
    addAndMakeVisible(orderValueLabel);
    updateOrderValue();

    orderDecButton.onClick = [this]
    {
        processor.setDisplayOrder(index, processor.getConfig(index).displayOrder - 1);
        processor.saveConfigToDisk();
        updateOrderValue();
    };
    addAndMakeVisible(orderDecButton);

    orderIncButton.onClick = [this]
    {
        processor.setDisplayOrder(index, processor.getConfig(index).displayOrder + 1);
        processor.saveConfigToDisk();
        updateOrderValue();
    };
    addAndMakeVisible(orderIncButton);
}

void InstrumentPanel::populateMidiCombo()
{
    for (int n = 0; n < 128; ++n)
    {
        const auto noteName = juce::MidiMessage::getMidiNoteName(n, true, true, 4);
        midiNoteCombo.addItem(noteName + "  (" + juce::String(n) + ")", n + 1);
    }
}

void InstrumentPanel::populateChokeCombo()
{
    chokeCombo.addItem("None", 1);
    for (int j = 0; j < 16; ++j)
        if (j != index)
            chokeCombo.addItem("Inst " + juce::String::formatted("%02d", j + 1), j + 2);
}

void InstrumentPanel::updateOrderValue()
{
    orderValueLabel.setText(juce::String(processor.getConfig(index).displayOrder),
                            juce::dontSendNotification);
}

void InstrumentPanel::paint(juce::Graphics& g)
{
    g.setColour(juce::Colour(0xff3a3a3a));
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 4.0f);

    g.setColour(juce::Colour(0xff555555));
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(0.5f), 4.0f, 1.0f);
}

void InstrumentPanel::resized()
{
    auto area = getLocalBounds().reduced(5);
    const int rowH = 22;
    const int gap  = 4;
    const int labelW = 38;

    // Row 0: index label + trigger button
    auto row0 = area.removeFromTop(24);
    indexLabel   .setBounds(row0.removeFromLeft(28));
    triggerButton.setBounds(row0.removeFromRight(74));

    area.removeFromTop(gap);

    // Row 1: name
    auto row1 = area.removeFromTop(rowH);
    nameLabel .setBounds(row1.removeFromLeft(labelW));
    nameEditor.setBounds(row1);

    area.removeFromTop(gap);

    // Row 2: MIDI note
    auto row2 = area.removeFromTop(rowH);
    midiLabel   .setBounds(row2.removeFromLeft(labelW));
    midiNoteCombo.setBounds(row2);

    area.removeFromTop(gap);

    // Row 3: choke
    auto row3 = area.removeFromTop(rowH);
    chokeLabel.setBounds(row3.removeFromLeft(labelW));
    chokeCombo.setBounds(row3);

    area.removeFromTop(gap);

    // Row 4: display order
    auto row4 = area.removeFromTop(rowH);
    orderLabel     .setBounds(row4.removeFromLeft(labelW));
    orderDecButton .setBounds(row4.removeFromLeft(22));
    row4.removeFromLeft(2);
    orderValueLabel.setBounds(row4.removeFromLeft(28));
    row4.removeFromLeft(2);
    orderIncButton .setBounds(row4.removeFromLeft(22));
}
