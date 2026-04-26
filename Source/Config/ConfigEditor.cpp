#include "ConfigEditor.h"
#include "ConfigProcessor.h"

static constexpr int PanelW    = 212;
static constexpr int PanelH    = 135;
static constexpr int PanelGap  = 10;
static constexpr int EdgePad   = 10;
static constexpr int BottomBar = 48;

ConfigEditor::ConfigEditor(ConfigProcessor& p)
    : AudioProcessorEditor(p), processor(p)
{
    for (int i = 0; i < 16; ++i)
    {
        panels[i] = std::make_unique<InstrumentPanel>(i, processor);
        addAndMakeVisible(*panels[i]);
    }

    saveButton.onClick = [this] { processor.saveConfigToDisk(); };
    addAndMakeVisible(saveButton);

    exportButton.onClick = [this]
    {
        fileChooser = std::make_unique<juce::FileChooser>(
            "Save C++ Header",
            juce::File::getSpecialLocation(juce::File::userDesktopDirectory)
                .getChildFile("InstrumentData.h"),
            "*.h");

        fileChooser->launchAsync(
            juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser& fc)
            {
                const auto dest = fc.getResult();
                if (dest != juce::File{})
                    dest.replaceWithText(processor.generateCppHeader());
            });
    };
    addAndMakeVisible(exportButton);

    // Window size derived from 4×4 grid + bottom bar
    const int w = EdgePad + 4 * PanelW + 3 * PanelGap + EdgePad;
    const int h = EdgePad + 4 * PanelH + 3 * PanelGap + EdgePad + PanelGap + BottomBar + EdgePad;
    setSize(w, h);
}

ConfigEditor::~ConfigEditor() {}

void ConfigEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff222222));
}

void ConfigEditor::resized()
{
    // Position panels in a 4×4 grid
    for (int i = 0; i < 16; ++i)
    {
        const int col = i % 4;
        const int row = i / 4;
        const int x   = EdgePad + col * (PanelW + PanelGap);
        const int y   = EdgePad + row * (PanelH + PanelGap);
        panels[i]->setBounds(x, y, PanelW, PanelH);
    }

    // Bottom bar
    const int barY = EdgePad + 4 * PanelH + 3 * PanelGap + EdgePad + PanelGap;
    saveButton  .setBounds(EdgePad,              barY, 140, 32);
    exportButton.setBounds(EdgePad + 150,        barY, 200, 32);
}
