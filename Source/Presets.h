#pragma once
#include <array>
#include <cstring>
#include <string>
#include <vector>

struct SynthParams
{
    float waveform { 0.0f }; // 0 saw, 1 pulse
    float tuning { 0.0f };   // -4..+4
    float cutoff { 0.40f };
    float reso { 0.45f };
    float envMod { 0.65f };
    float decay { 0.40f };
    float accent { 0.55f };
    float volume { 0.70f };
    float tempo { 130.0f };
    float seqOn { 1.0f };
};

struct FactoryPreset
{
    const char* name;
    SynthParams params;
};

inline const std::array<FactoryPreset, 10>& getFactoryPresets()
{
    static const std::array<FactoryPreset, 10> presets {{
        { "init",      { 0, 0, 0.40f, 0.35f, 0.50f, 0.45f, 0.40f, 0.70f, 130, 1 } },
        { "me acid 1", { 0, 0, 0.28f, 0.72f, 0.85f, 0.28f, 0.70f, 0.75f, 135, 1 } },
        { "me acid 2", { 1, 0, 0.22f, 0.80f, 0.90f, 0.22f, 0.75f, 0.72f, 128, 1 } },
        { "me acid 3", { 0, -1, 0.35f, 0.60f, 0.70f, 0.50f, 0.55f, 0.70f, 140, 1 } },
        { "me acid 4", { 1, 1, 0.18f, 0.88f, 0.95f, 0.18f, 0.85f, 0.70f, 125, 1 } },
        { "me acid 5", { 0, 0, 0.45f, 0.50f, 0.55f, 0.60f, 0.45f, 0.72f, 132, 1 } },
        { "me acid 6", { 1, -2, 0.30f, 0.75f, 0.80f, 0.35f, 0.65f, 0.74f, 138, 1 } },
        { "me acid 7", { 0, 2, 0.25f, 0.68f, 0.78f, 0.25f, 0.80f, 0.70f, 120, 1 } },
        { "me acid 8", { 1, 0, 0.40f, 0.55f, 0.60f, 0.42f, 0.50f, 0.73f, 145, 1 } },
        { "me acid 9", { 0, 0, 0.15f, 0.92f, 1.00f, 0.15f, 0.90f, 0.68f, 130, 1 } },
    }};
    return presets;
}
