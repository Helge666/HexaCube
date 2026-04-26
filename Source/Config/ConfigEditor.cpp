#include "ConfigEditor.h"

ConfigEditor::ConfigEditor(ConfigProcessor& p)
    : AudioProcessorEditor(p), processor(p)
{
    setSize(900, 600);
}

ConfigEditor::~ConfigEditor() {}

void ConfigEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);
    g.setColour(juce::Colours::white);
    g.setFont(24.0f);
    g.drawFittedText("HexaCube Config", getLocalBounds(), juce::Justification::centred, 1);
}

void ConfigEditor::resized() {}
