#pragma once
#include <cmath>
#include <algorithm>
#include <array>

/** 4-step bipolar CV sequencer with its own clock division and one-pole lag. */
class CvStepper
{
public:
    // beats per step for each division index
    static constexpr int kNumDivisions = 9;
    static float beatsForDivision (int idx) noexcept
    {
        static constexpr float beats[kNumDivisions] = {
            0.125f,       // 1/32
            1.0f / 6.0f,  // 1/16T
            0.25f,        // 1/16
            1.0f / 3.0f,  // 1/8T
            0.5f,         // 1/8
            2.0f / 3.0f,  // 1/4T
            1.0f,         // 1/4
            2.0f,         // 1/2
            4.0f          // 1/1 (one measure)
        };
        return beats[std::clamp (idx, 0, kNumDivisions - 1)];
    }

    void prepare (double sr) noexcept
    {
        sampleRate = sr;
        samplesIntoStep = 0;
        stepIndex = 0;
        smoothed = 0.0f;
        target = 0.0f;
    }

    void setTempo (double bpm) noexcept { tempoBpm = std::clamp (bpm, 40.0, 300.0); }
    void setDivision (int idx) noexcept { division = std::clamp (idx, 0, kNumDivisions - 1); }

    void setRunning (bool r) noexcept
    {
        if (r && ! running)
        {
            stepIndex = 0;
            samplesIntoStep = 0;
            target = steps[0];
            if (lagAmt < 0.001f) smoothed = target;
        }
        running = r;
    }

    void setStepValue (int i, float v) noexcept
    {
        if (i >= 0 && i < 4)
            steps[(size_t) i] = std::clamp (v, -1.0f, 1.0f);
    }

    void setLag (float lag01) noexcept { lagAmt = std::clamp (lag01, 0.0f, 1.0f); }

    int getStepIndex() const noexcept { return stepIndex; }
    float getSmoothed() const noexcept { return smoothed; }
    float getTarget() const noexcept { return target; }

    int samplesPerStep() const noexcept
    {
        const double beats = (double) beatsForDivision (division);
        const double stepSec = (60.0 / tempoBpm) * beats;
        return std::max (1, (int) std::round (stepSec * sampleRate));
    }

    /** Advance one sample; returns true when a new step latches. */
    bool advance() noexcept
    {
        bool trig = false;
        if (running)
        {
            const int stepLen = samplesPerStep();
            if (samplesIntoStep == 0)
            {
                target = steps[(size_t) stepIndex];
                if (lagAmt < 0.001f)
                    smoothed = target;
                trig = true;
            }
            ++samplesIntoStep;
            if (samplesIntoStep >= stepLen)
            {
                samplesIntoStep = 0;
                stepIndex = (stepIndex + 1) & 3;
            }
        }

        // Lag: 0 = instant, 1 ~= 500 ms one-pole toward target
        if (lagAmt < 0.001f)
        {
            smoothed = target;
        }
        else
        {
            const float lagSec = 0.002f + lagAmt * lagAmt * 0.55f; // ~2 ms .. ~550 ms
            const float coeff = (float) std::exp (-1.0 / (lagSec * sampleRate));
            smoothed = target + (smoothed - target) * coeff;
        }
        return trig;
    }

private:
    double sampleRate { 44100.0 };
    double tempoBpm { 130.0 };
    int division { 2 }; // default 1/16
    bool running { false };
    int stepIndex { 0 };
    int samplesIntoStep { 0 };
    float lagAmt { 0 };
    float smoothed { 0 }, target { 0 };
    std::array<float, 4> steps { 0, 0, 0, 0 };
};
