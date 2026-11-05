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
        phase2 = 0.0;
        fmEnv = 0.0f;
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

    // Modulator oscillator (Osc 2): never reaches the audio output
    void setModWaveTriangle (bool tri) noexcept { modTri = tri; }
    void setModOctave (int oct) noexcept { modOct = std::clamp (oct, -2, 2); }
    void setModSync (float s01) noexcept { modSync = std::clamp (s01, 0.0f, 1.0f); }
    void setFmVco (float f01) noexcept { fmVco = std::clamp (f01, 0.0f, 1.0f); }
    void setFmVcf (float f01) noexcept { fmVcf = std::clamp (f01, 0.0f, 1.0f); }
    void setFmEnvAmt (float a01) noexcept { fmEnvAmt = std::clamp (a01, 0.0f, 1.0f); }
    void setFmDecay (float d01) noexcept { fmDecay = std::clamp (d01, 0.0f, 1.0f); }

    void setBendSemitones (float b) noexcept
    {
        bendSemis = b;
        if (curNote >= 0)
        {
            targetHz = 440.0 * std::pow (2.0, ((double) curNote - 69.0 + tuningSemis + bendSemis) / 12.0);
            if (slideRemain == 0) currentHz = targetHz;
        }
    }

    void noteOn (int midiNote, bool accent, bool slide) noexcept
    {
        curNote = midiNote;
        const float semis = (float) midiNote - 69.0f + tuningSemis + bendSemis;
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
        fmEnv = 1.0f; // FM envelope retriggers on every note
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

        // FM envelope (exponential decay, 15 ms .. 1.5 s), accent adds a bit of depth
        const float fmDecSec = 0.015f + fmDecay * fmDecay * 1.5f;
        fmEnv *= (float) std::exp (-1.0 / (fmDecSec * sampleRate));
        const float accBoost = accented ? (1.0f + 0.5f * accentAmt) : 1.0f;
        const float envPart = fmEnvAmt * fmEnv * accBoost;
        const float depthVco = std::min (2.0f, fmVco * 1.0f + envPart * 1.0f);
        const float depthVcf = std::min (2.0f, fmVcf * 1.0f + envPart * 1.0f);

        // Modulator oscillator: locked to the played pitch (incl. slide and bend), octave offset
        const double freq = std::clamp (currentHz, 20.0, sampleRate * 0.45);
        const double freq2 = std::clamp (freq * std::pow (2.0, (double) modOct), 5.0, sampleRate * 0.25);
        const double dt2 = freq2 / sampleRate;
        float mod = 0.0f;
        if (modTri) mod = (float) (1.0 - 4.0 * std::fabs (phase2 - 0.5));
        else        mod = (float) std::sin (6.283185307179586 * phase2);
        phase2 += dt2;
        if (phase2 >= 1.0) phase2 -= 1.0;

        // Main oscillator (polyBLEP saw / square) with linear VCO FM from the modulator
        const double fmScale = 1.0 + (double) (depthVco * mod);
        const double freqFm = std::clamp (freq * std::max (0.02, fmScale), 5.0, sampleRate * 0.45);
        const double dt = freqFm / sampleRate;
        float osc = usePulse ? polyBlepSquare (phase, dt) : polyBlepSaw (phase, dt);
        phase += dt;
        if (phase >= 1.0)
        {
            phase -= 1.0;
            // Hard sync: restart the modulator when the main oscillator wraps (blend by Sync amount)
            phase2 *= (1.0 - (double) modSync);
        }

        if (clickRemain > 0)
        {
            osc += (float) ((clickRemain & 1) ? 0.6 : -0.6) * accentAmt;
            --clickRemain;
        }

        // Filter cutoff mapping
        const float base = 40.0f * std::pow (2.0f, cutoff * 9.0f); // ~40 Hz .. ~20 kHz
        const float modEnv = filtEnv * envMod * (3500.0f + 2500.0f * (accented ? accentAmt : 0.0f));
        const float accentOpen = accented ? (800.0f * accentAmt * filtEnv) : 0.0f;
        // VCF FM: modulator moves the cutoff up to +/-3 octaves at full depth
        const float vcfFm = std::pow (2.0f, 3.0f * depthVcf * mod);
        filter.setCutoffHz ((double) ((base + modEnv + accentOpen) * vcfFm));
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
    double phase { 0.0 }, phase2 { 0.0 };
    float fmEnv { 0 }, fmVco { 0 }, fmVcf { 0 }, fmEnvAmt { 0 }, fmDecay { 0.3f }, modSync { 0 };
    int modOct { 0 };
    bool modTri { false };
    double currentHz { 110.0 }, targetHz { 110.0 }, slideStartHz { 110.0 };
    float ampEnv { 0 }, filtEnv { 0 };
    int curNote { -1 };
    float bendSemis { 0 };
    float tuningSemis { 0 }, cutoff { 0.4f }, reso { 0.4f }, envMod { 0.6f };
    float decay { 0.4f }, accentAmt { 0.5f }, volume { 0.7f };
    bool usePulse { false }, gateOn { false }, accented { false };
    int slideSamples { 2646 }, slideRemain { 0 }, clickRemain { 0 };
    LadderFilter filter;
};
