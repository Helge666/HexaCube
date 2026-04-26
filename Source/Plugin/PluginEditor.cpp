#include "PluginEditor.h"

HexaCubeEditor::HexaCubeEditor(HexaCubeProcessor& p)
    : AudioProcessorEditor(p), processor(p)
{
    setSize(800, 500);
}

HexaCubeEditor::~HexaCubeEditor() {}

void HexaCubeEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);
    g.setColour(juce::Colours::white);
    g.setFont(24.0f);
    g.drawFittedText("HexaCube", getLocalBounds(), juce::Justification::centred, 1);
}

void HexaCubeEditor::resized() {}
