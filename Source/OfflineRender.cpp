// Headless smoke test: render one bar of the default 16-step pattern to WAV.
#include <algorithm>
#include "VoiceEngine.h"
#include "Sequencer.h"
#include "Presets.h"
#include <cstdio>
#include <cstdint>
#include <vector>
#include <cstdlib>
#include <string>
#include <cmath>

static void writeWav (const std::string& path, const std::vector<float>& mono, int sr)
{
    const int n = (int) mono.size();
    const int dataBytes = n * 2;
    FILE* f = std::fopen (path.c_str(), "wb");
    if (! f) { std::perror ("fopen"); return; }
    auto w32 = [&] (uint32_t v) { std::fwrite (&v, 4, 1, f); };
    auto w16 = [&] (uint16_t v) { std::fwrite (&v, 2, 1, f); };
    std::fwrite ("RIFF", 1, 4, f);
    w32 (36 + (uint32_t) dataBytes);
    std::fwrite ("WAVEfmt ", 1, 8, f);
    w32 (16); w16 (1); w16 (1); w32 ((uint32_t) sr); w32 ((uint32_t) sr * 2); w16 (2); w16 (16);
    std::fwrite ("data", 1, 4, f);
    w32 ((uint32_t) dataBytes);
    for (float s : mono)
    {
        float c = std::tanh (s);
        int16_t i = (int16_t) std::clamp (c * 30000.0f, -32768.0f, 32767.0f);
        std::fwrite (&i, 2, 1, f);
    }
    std::fclose (f);
}

int main (int argc, char** argv)
{
    const char* out = argc > 1 ? argv[1] : "/workspace/lunchbox303/sample_pattern.wav";
    const double sr = 44100.0;
    const auto& preset = getFactoryPresets()[argc > 2 ? std::atoi (argv[2]) : 1];

    VoiceEngine voice;
    Sequencer seq;
    voice.prepare (sr);
    seq.prepare (sr);
    voice.setWaveform (preset.params.waveform > 0.5f);
    voice.setTuningSemitones (preset.params.tuning);
    voice.setCutoff (preset.params.cutoff);
    voice.setResonance (preset.params.reso);
    voice.setEnvMod (preset.params.envMod);
    voice.setDecay (preset.params.decay);
    voice.setAccentAmt (preset.params.accent);
    voice.setVolume (preset.params.volume);
    voice.setModWaveTriangle (preset.params.modWave > 0.5f);
    voice.setModOctave ((int) preset.params.modOct - 2);
    voice.setModSync (preset.params.sync);
    voice.setFmVco (preset.params.fmVco);
    voice.setFmVcf (preset.params.fmVcf);
    voice.setFmEnvAmt (preset.params.fmEnv);
    voice.setFmDecay (preset.params.fmDecay);
    seq.setTempo (preset.params.tempo);
    seq.setRunning (true);

    const int total = (int) (sr * 4.0); // ~4 seconds
    std::vector<float> buf ((size_t) total);
    Step cur {};
    bool gated = false;
    for (int i = 0; i < total; ++i)
    {
        Step step;
        const int into = seq.getSamplesIntoStep();
        const bool trig = seq.advance (step);
        if (trig)
        {
            cur = step;
            voice.noteOn (step.noteNumber, step.accent, step.slide);
            gated = true;
        }
        const int half = seq.samplesPerStep() / 2;
        if (gated && into == half && ! cur.slide)
        {
            voice.noteOff();
            gated = false;
        }
        buf[(size_t) i] = voice.process();
    }
    writeWav (out, buf, (int) sr);
    std::printf ("Wrote %s (%d samples)\n", out, total);
    return 0;
}
