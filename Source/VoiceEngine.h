#pragma once
#include "LadderFilter.h"
#include <cmath>
#include <algorithm>

class VoiceEngine
{
public:
    void prepare (double sr) noexcept
    {
        sampleRate = sr;
        filter.setSampleRate (sr);
        filter.reset();
        slideSamples = (int) std::round (0.060 * sr); // ~60 ms
        reset();
    }

    void reset() noexcept
    {
        phase = 0.0;
        ampEnv = 0.0;
        filtEnv = 0.0;
        currentHz = targetHz = 110.0;
        slideRemain = 0;
        gateOn = false;
        accented = false;
        filter.reset();
    }

    void setWaveform (bool pulse) noexcept { usePulse = pulse; }
    void setTuningSemitones (float s) noexcept { tuningSemis = std::clamp (s, -4.0f, 4.0f); }
    void setCutoff (float c01) noexcept { cutoff = std::clamp (c01, 0.0f, 1.0f); }
    void setResonance (float r01) noexcept { reso = std::clamp (r01, 0.0f, 1.0f); }
    void setEnvMod (float e01) noexcept { envMod = std::clamp (e01, 0.0f, 1.0f); }
    void setDecay (float d01) noexcept { decay = std::clamp (d01, 0.0f, 1.0f); }
    void setAccentAmt (float a01) noexcept { accentAmt = std::clamp (a01, 0.0f, 1.0f); }
    void setVolume (float v01) noexcept { volume = std::clamp (v01, 0.0f, 1.0f); }

    void noteOn (int midiNote, bool accent, bool slide) noexcept
    {
        const float semis = (float) midiNote - 69.0f + tuningSemis;
        targetHz = 440.0 * std::pow (2.0, semis / 12.0);

        if (slide && gateOn)
        {
            slideRemain = slideSamples;
            slideStartHz = currentHz;
        }
        else
        {
            currentHz = targetHz;
            slideRemain = 0;
            phase = 0.0;
        }

        accented = accent;
        gateOn = true;

        // Accent: brighter / louder / slightly faster decay feel
        const float a = accent ? accentAmt : 0.0f;
        ampEnv = 0.85f + 0.35f * a;
        filtEnv = 1.0f;
        clickRemain = accent ? (int) (0.003 * sampleRate) : 0; // short click
    }

    void noteOff() noexcept
    {
        gateOn = false;
    }

    float process() noexcept
    {
        if (slideRemain > 0)
        {
            const float t = 1.0f - (float) slideRemain / (float) slideSamples;
            currentHz = slideStartHz + (targetHz - slideStartHz) * t;
            --slideRemain;
        }
        else
        {
            currentHz = targetHz;
        }

        // Envelopes
        const float decSec = 0.05f + decay * 1.2f;
        const float aBoost = accented ? (0.35f * accentAmt) : 0.0f;
        const float filtDecSec = std::max (0.02f, decSec * (1.0f - 0.4f * aBoost));
        const float ampDec = (float) std::exp (-1.0 / (filtDecSec * sampleRate));
        const float fltDec = (float) std::exp (-1.0 / (filtDecSec * 0.85 * sampleRate));

        if (gateOn)
        {
            // while gated, amp holds with slight decay (303-ish)
            ampEnv *= 0.9997f + 0.0002f * decay;
            if (ampEnv < 0.4f) ampEnv = 0.4f;
        }
        else
        {
            ampEnv *= ampDec;
        }
        filtEnv *= fltDec;

        // Oscillator (polyBLEP saw / square)
        const double freq = std::clamp (currentHz, 20.0, sampleRate * 0.45);
        const double dt = freq / sampleRate;
        float osc = usePulse ? polyBlepSquare (phase, dt) : polyBlepSaw (phase, dt);
        phase += dt;
        if (phase >= 1.0) phase -= 1.0;

        if (clickRemain > 0)
        {
            osc += (float) ((clickRemain & 1) ? 0.6 : -0.6) * accentAmt;
            --clickRemain;
        }

        // Filter cutoff mapping
        const float base = 40.0f * std::pow (2.0f, cutoff * 9.0f); // ~40 Hz .. ~20 kHz
        const float mod = filtEnv * envMod * (3500.0f + 2500.0f * (accented ? accentAmt : 0.0f));
        const float accentOpen = accented ? (800.0f * accentAmt * filtEnv) : 0.0f;
        filter.setCutoffHz ((double) (base + mod + accentOpen));
        filter.setResonance ((double) (0.05f + reso * 0.92f));

        float y = filter.process (osc * 1.4f);
        y = std::tanh (y * (1.1f + 0.5f * reso));
        const float lev = ampEnv * (0.55f + 0.45f * (accented ? (0.5f + 0.5f * accentAmt) : 0.7f));
        return y * lev * volume * 0.7f;
    }

    bool isActive() const noexcept { return ampEnv > 0.0005f || gateOn; }

private:
    static float polyBlep (double t, double dt) noexcept
    {
        if (t < dt)
        {
            const double x = t / dt;
            return (float) (x + x - x * x - 1.0);
        }
        if (t > 1.0 - dt)
        {
            const double x = (t - 1.0) / dt;
            return (float) (x * x + x + x + 1.0);
        }
        return 0.0f;
    }

    static float polyBlepSaw (double phase, double dt) noexcept
    {
        float y = (float) (2.0 * phase - 1.0);
        y -= polyBlep (phase, dt);
        return y;
    }

    static float polyBlepSquare (double phase, double dt) noexcept
    {
        float y = phase < 0.5 ? 1.0f : -1.0f;
        y += polyBlep (phase, dt);
        double t2 = phase + 0.5;
        if (t2 >= 1.0) t2 -= 1.0;
        y -= polyBlep (t2, dt);
        return y;
    }

    double sampleRate { 44100.0 };
    double phase { 0.0 };
    double currentHz { 110.0 }, targetHz { 110.0 }, slideStartHz { 110.0 };
    float ampEnv { 0 }, filtEnv { 0 };
    float tuningSemis { 0 }, cutoff { 0.4f }, reso { 0.4f }, envMod { 0.6f };
    float decay { 0.4f }, accentAmt { 0.5f }, volume { 0.7f };
    bool usePulse { false }, gateOn { false }, accented { false };
    int slideSamples { 2646 }, slideRemain { 0 }, clickRemain { 0 };
    LadderFilter filter;
};
