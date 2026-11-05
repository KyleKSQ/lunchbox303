#pragma once
#include <cmath>
#include <algorithm>

/** Nonlinear 4-pole diode-ladder–style lowpass (Huovilainen-inspired). */
class LadderFilter
{
public:
    void reset() noexcept
    {
        for (auto& s : stage) s = 0.0;
        for (auto& z : delay) z = 0.0;
    }

    void setSampleRate (double sr) noexcept
    {
        sampleRate = sr > 1.0 ? sr : 44100.0;
    }

    void setCutoffHz (double hz) noexcept
    {
        cutoffHz = std::clamp (hz, 20.0, sampleRate * 0.45);
    }

    void setResonance (double r) noexcept
    {
        resonance = std::clamp (r, 0.0, 1.0);
    }

    float process (float in) noexcept
    {
        const double fc = cutoffHz / sampleRate;
        double f = fc * 1.16;
        f = std::clamp (f, 0.0001, 0.99);
        const double f2 = f * f;
        const double feedback = resonance * (1.0 - 0.15 * f2) * 4.5;

        double x = (double) in - feedback * delay[3];
        x = std::tanh (x);

        for (int i = 0; i < 4; ++i)
        {
            const double p = f * 2.0;
            const double t = 1.0 / (1.0 + p);
            const double y = (x + stage[i]) * t;
            stage[i] = (y + x) * p + y; // bilinear-ish one-pole cascade
            x = std::tanh (y);
            delay[i] = x;
        }
        return (float) x;
    }

private:
    double sampleRate { 44100.0 };
    double cutoffHz { 1000.0 };
    double resonance { 0.3 };
    double stage[4] {};
    double delay[4] {};
};
