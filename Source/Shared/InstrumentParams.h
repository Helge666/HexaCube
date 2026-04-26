#pragma once

struct InstrumentParams
{
    float volume = 1.0f;  // 0.0 – 1.0
    float pan    = 0.0f;  // -1.0 (L) – 1.0 (R)
    bool  mute   = false;
};
