#include "PluginEditor.h"
#include "Shared/InstrumentData.h"

HexaCubeEditor::HexaCubeEditor(HexaCubeProcessor& p)
    : AudioProcessorEditor(p), processor(p)
{
    for (int pos = 0; pos < 16; ++pos)
    {
        strips[pos] = std::make_unique<InstrumentStrip>(p, DISPLAY_ORDER[pos]);
        addAndMakeVisible(*strips[pos]);
    }

    setSize(828, 560);
}

HexaCubeEditor::~HexaCubeEditor() {}

void HexaCubeEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff222222));
}

void HexaCubeEditor::resized()
{
    constexpr int margin = 8;
    constexpr int gap    = 4;
    constexpr int cols   = 4;
    constexpr int rows   = 4;

    const int panelW = (getWidth()  - 2 * margin - (cols - 1) * gap) / cols;
    const int panelH = (getHeight() - 2 * margin - (rows - 1) * gap) / rows;

    for (int pos = 0; pos < 16; ++pos)
    {
        const int col = pos % cols;
        const int row = pos / cols;
        strips[pos]->setBounds(margin + col * (panelW + gap),
                               margin + row * (panelH + gap),
                               panelW, panelH);
    }
}
