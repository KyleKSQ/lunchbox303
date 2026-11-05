#pragma once
#include <array>
#include <cstdint>
#include <cmath>
#include <algorithm>

struct Step
{
    int noteNumber { 36 }; // C2 default
    bool gate { true };
    bool accent { false };
    bool slide { false };
};

class Sequencer
{
public:
    std::array<Step, 16> steps {};

    Sequencer()
    {
        // Default acid-ish pattern
        const int n[16] = { 36,36,48,36, 39,36,43,36, 36,48,36,41, 36,39,43,48 };
        const bool g[16] = { 1,0,1,1, 1,0,1,1, 1,1,0,1, 1,0,1,1 };
        const bool a[16] = { 1,0,0,0, 1,0,0,1, 0,1,0,0, 1,0,0,1 };
        const bool s[16] = { 0,0,1,0, 0,0,0,1, 0,0,0,1, 0,0,1,0 };
        for (int i = 0; i < 16; ++i)
        {
            steps[(size_t) i] = { n[i], g[i], a[i], s[i] };
        }
    }

    void setTempo (double bpm) noexcept { tempoBpm = std::clamp (bpm, 40.0, 300.0); }
    double getTempo() const noexcept { return tempoBpm; }

    void setRunning (bool r) noexcept
    {
        if (r && ! running)
        {
            stepIndex = 0;
            samplesIntoStep = 0;
            justTriggered = false;
        }
        running = r;
    }

    bool isRunning() const noexcept { return running; }
    int getStepIndex() const noexcept { return stepIndex; }

    void prepare (double sr) noexcept { sampleRate = sr; }

    /** Advance one audio sample; returns true if a new gated step triggered. */
    bool advance (Step& outStep) noexcept
    {
        justTriggered = false;
        if (! running)
            return false;

        const double stepSec = (60.0 / tempoBpm) / 4.0; // 16th notes
        const int stepLen = std::max (1, (int) std::round (stepSec * sampleRate));

        if (samplesIntoStep == 0)
        {
            outStep = steps[(size_t) stepIndex];
            justTriggered = outStep.gate;
        }

        ++samplesIntoStep;
        if (samplesIntoStep >= stepLen)
        {
            samplesIntoStep = 0;
            stepIndex = (stepIndex + 1) & 15;
        }
        return justTriggered && samplesIntoStep == 1;
    }

    bool shouldGateOff (int samplesPerGate) const noexcept
    {
        // gate length ~ half step by default
        return samplesIntoStep == samplesPerGate;
    }

    int samplesPerStep() const noexcept
    {
        const double stepSec = (60.0 / tempoBpm) / 4.0;
        return std::max (1, (int) std::round (stepSec * sampleRate));
    }

    int getSamplesIntoStep() const noexcept { return samplesIntoStep; }

private:
    double sampleRate { 44100.0 };
    double tempoBpm { 130.0 };
    bool running { false };
    int stepIndex { 0 };
    int samplesIntoStep { 0 };
    bool justTriggered { false };
};
