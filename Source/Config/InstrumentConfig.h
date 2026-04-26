#pragma once
#include <JuceHeader.h>

struct InstrumentConfig
{
    juce::String name        { "Instrument" };
    int          midiNote    { 36 };   // MIDI 36 = C2 default, overridden in ConfigProcessor ctor
    int          chokeTarget { -1 };   // -1 = none; instrument index otherwise
    int          displayOrder{ 1 };    // 1–16; set to (index + 1) by default
};
