#pragma once
#include <cmath>
#include <algorithm>

/** 4-pole resonant lowpass (Moog/Stilson-Smith style ladder with tanh saturation,
    zero-delay-ish one-pole stages and 2x feedback compensation). Stable for all settings. */
class LadderFilter
{
public:
    void reset() noexcept
    {
        for (auto& s : s_) s = 0.0;
    }

    void setSampleRate (double sr) noexcept { sampleRate = sr > 1.0 ? sr : 44100.0; }
    void setCutoffHz (double hz) noexcept { cutoffHz = std::clamp (hz, 20.0, sampleRate * 0.45); }
    void setResonance (double r) noexcept { resonance = std::clamp (r, 0.0, 1.0); }

    float process (float in) noexcept
    {
        // Trapezoidal one-pole coefficient (TPT): g = tan(pi*fc/fs)
        const double g = std::tan (3.14159265358979323846 * cutoffHz / sampleRate);
        const double G = g / (1.0 + g);
        const double k = resonance * 3.9; // < 4 keeps loop below hard self-oscillation

        // Solve the feedback loop (linear approximation) to avoid a unit delay
        const double G4 = G * G * G * G;
        const double sigma = (G * G * G * s_[0] + G * G * s_[1] + G * s_[2] + s_[3]) ;
        double u = ((double) in - k * sigma / (1.0 + g)) / (1.0 + k * G4);
        u = std::tanh (u);

        double x = u;
        for (int i = 0; i < 4; ++i)
        {
            const double v = (x - s_[i]) * G;
            const double y = v + s_[i];
            s_[i] = y + v;
            x = y;
        }
        // tame resonance peaks softly
        return (float) std::tanh (x);
    }

private:
    double sampleRate { 44100.0 };
    double cutoffHz { 1000.0 };
    double resonance { 0.3 };
    double s_[4] {};
};
