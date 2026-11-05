#include "PluginProcessor.h"
#include <algorithm>
#include "PluginEditor.h"

namespace
{
    const char* kCvDestIds[] = {
        "None", "Tuning", "Cut Off", "Resonance", "Env Mod", "Decay", "Accent", "Volume",
        "Sync", "VCO FM", "VCF FM", "FM Env", "FM Decay", "Wavefold", "Glide"
    };
    constexpr int kNumCvDests = 15;
}

const juce::StringArray& Lunchbox303AudioProcessor::cvDestChoices()
{
    static juce::StringArray choices;
    if (choices.isEmpty())
        for (int i = 0; i < kNumCvDests; ++i)
            choices.add (kCvDestIds[i]);
    return choices;
}

Lunchbox303AudioProcessor::Lunchbox303AudioProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMS", createParameterLayout())
{
    setCurrentProgram (0);
}

juce::AudioProcessorValueTreeState::ParameterLayout Lunchbox303AudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("waveform", "SAW/PULSE", juce::StringArray { "SAW", "PULSE" }, 0));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("tuning", "TUNING", juce::NormalisableRange<float> (-4.0f, 4.0f, 0.01f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("cutoff", "CUT OFF", juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.4f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("reso", "RESO", juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.45f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("envmod", "ENV MOD", juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.65f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("decay", "DECAY", juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.4f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("accent", "ACCENT", juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.55f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("volume", "VOLUME", juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.7f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("wavefold", "WAVEFOLD", juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("glide", "GLIDE", juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.3f));
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("modwave", "OSC2 WAVE", juce::StringArray { "SINE", "TRI" }, 0));
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("modoct", "OSC2 OCTAVE", juce::StringArray { "-2", "-1", "0", "+1", "+2" }, 2));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("sync", "SYNC", juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("fmvco", "VCO FM", juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("fmvcf", "VCF FM", juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("fmenv", "FM ENV", juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("fmdecay", "FM DECAY", juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.3f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("tempo", "BPM", juce::NormalisableRange<float> (40.0f, 300.0f, 0.1f), 130.0f));
    params.push_back (std::make_unique<juce::AudioParameterBool> ("seqon", "SEQ", false));
    params.push_back (std::make_unique<juce::AudioParameterBool> ("hostsync", "HOST SYNC", false));

    // CV stepper
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("cv1", "CV 1", juce::NormalisableRange<float> (-1.0f, 1.0f, 0.001f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("cv2", "CV 2", juce::NormalisableRange<float> (-1.0f, 1.0f, 0.001f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("cv3", "CV 3", juce::NormalisableRange<float> (-1.0f, 1.0f, 0.001f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("cv4", "CV 4", juce::NormalisableRange<float> (-1.0f, 1.0f, 0.001f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("cvlag", "CV LAG", juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterBool> ("cvon", "CV", true));
    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        "cvdiv", "CV DIV",
        juce::StringArray { "1/32", "1/16T", "1/16", "1/8T", "1/8", "1/4T", "1/4", "1/2", "1/1" },
        2)); // default 1/16

    auto& dests = cvDestChoices();
    for (int i = 1; i <= 4; ++i)
    {
        const auto n = juce::String (i);
        params.push_back (std::make_unique<juce::AudioParameterChoice> ("cvdest" + n, "CV DEST " + n, dests, 0));
        params.push_back (std::make_unique<juce::AudioParameterFloat> ("cvmin" + n, "CV MIN " + n, juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.0f));
        params.push_back (std::make_unique<juce::AudioParameterFloat> ("cvmax" + n, "CV MAX " + n, juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 1.0f));
    }

    return { params.begin(), params.end() };
}

void Lunchbox303AudioProcessor::prepareToPlay (double sampleRate, int)
{
    voice.prepare (sampleRate);
    sequencer.prepare (sampleRate);
    cvStepper.prepare (sampleRate);
}

bool Lunchbox303AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    return true;
}

void Lunchbox303AudioProcessor::applyParamsToVoice()
{
    const int wave = (int) apvts.getRawParameterValue ("waveform")->load();
    voice.setWaveform (wave == 1);
    voice.setTuningSemitones (apvts.getRawParameterValue ("tuning")->load());
    voice.setCutoff (apvts.getRawParameterValue ("cutoff")->load());
    voice.setResonance (apvts.getRawParameterValue ("reso")->load());
    voice.setEnvMod (apvts.getRawParameterValue ("envmod")->load());
    voice.setDecay (apvts.getRawParameterValue ("decay")->load());
    voice.setAccentAmt (apvts.getRawParameterValue ("accent")->load());
    voice.setVolume (apvts.getRawParameterValue ("volume")->load());
    voice.setWavefold (apvts.getRawParameterValue ("wavefold")->load());
    voice.setGlide (apvts.getRawParameterValue ("glide")->load());
    voice.setModWaveTriangle ((int) apvts.getRawParameterValue ("modwave")->load() == 1);
    voice.setModOctave ((int) apvts.getRawParameterValue ("modoct")->load() - 2);
    voice.setModSync (apvts.getRawParameterValue ("sync")->load());
    voice.setFmVco (apvts.getRawParameterValue ("fmvco")->load());
    voice.setFmVcf (apvts.getRawParameterValue ("fmvcf")->load());
    voice.setFmEnvAmt (apvts.getRawParameterValue ("fmenv")->load());
    voice.setFmDecay (apvts.getRawParameterValue ("fmdecay")->load());

    const bool hostSync = apvts.getRawParameterValue ("hostsync")->load() > 0.5f;
    double bpm = apvts.getRawParameterValue ("tempo")->load();
    if (hostSync)
    {
        if (auto* playHead = getPlayHead())
        {
            if (auto pos = playHead->getPosition())
            {
                if (auto b = pos->getBpm())
                    bpm = *b;
            }
        }
    }
    sequencer.setTempo (bpm);
    cvStepper.setTempo (bpm);

    cvStepper.setStepValue (0, apvts.getRawParameterValue ("cv1")->load());
    cvStepper.setStepValue (1, apvts.getRawParameterValue ("cv2")->load());
    cvStepper.setStepValue (2, apvts.getRawParameterValue ("cv3")->load());
    cvStepper.setStepValue (3, apvts.getRawParameterValue ("cv4")->load());
    cvStepper.setLag (apvts.getRawParameterValue ("cvlag")->load());
    cvStepper.setDivision ((int) apvts.getRawParameterValue ("cvdiv")->load());
}

void Lunchbox303AudioProcessor::applyCvOverrides (float smoothedCv)
{
    // Dest index: 0=None .. 13=Wavefold, 14=Glide
    for (int slot = 1; slot <= 4; ++slot)
    {
        const auto n = juce::String (slot);
        const int dest = (int) apvts.getRawParameterValue ("cvdest" + n)->load();
        if (dest <= 0) continue;

        const float mn = apvts.getRawParameterValue ("cvmin" + n)->load();
        const float mx = apvts.getRawParameterValue ("cvmax" + n)->load();
        const float v = juce::jmap (smoothedCv, -1.0f, 1.0f, mn, mx); // 0..1 of param range (or inverted)

        switch (dest)
        {
            case 1: voice.setTuningSemitones (juce::jmap (v, 0.0f, 1.0f, -4.0f, 4.0f)); break;
            case 2: voice.setCutoff (v); break;
            case 3: voice.setResonance (v); break;
            case 4: voice.setEnvMod (v); break;
            case 5: voice.setDecay (v); break;
            case 6: voice.setAccentAmt (v); break;
            case 7: voice.setVolume (v); break;
            case 8: voice.setModSync (v); break;
            case 9: voice.setFmVco (v); break;
            case 10: voice.setFmVcf (v); break;
            case 11: voice.setFmEnvAmt (v); break;
            case 12: voice.setFmDecay (v); break;
            case 13: voice.setWavefold (v); break;
            case 14: voice.setGlide (v); break;
            default: break;
        }
    }
}

void Lunchbox303AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();
    applyParamsToVoice();

    const bool seqOn = apvts.getRawParameterValue ("seqon")->load() > 0.5f;
    const bool hostSync = apvts.getRawParameterValue ("hostsync")->load() > 0.5f;
    const bool cvOn = apvts.getRawParameterValue ("cvon")->load() > 0.5f;

    bool playing = true;
    if (hostSync)
    {
        playing = false;
        if (auto* playHead = getPlayHead())
            if (auto pos = playHead->getPosition())
                playing = pos->getIsPlaying();
        sequencer.setRunning (seqOn && playing);
        cvStepper.setRunning (cvOn && playing);
    }
    else
    {
        sequencer.setRunning (seqOn);
        cvStepper.setRunning (cvOn);
    }

    auto* left = buffer.getWritePointer (0);
    auto* right = buffer.getNumChannels() > 1 ? buffer.getWritePointer (1) : nullptr;
    const int n = buffer.getNumSamples();
    const int halfGate = std::max (1, sequencer.samplesPerStep() / 2);

    auto handleMidi = [this] (const juce::MidiMessage& msg)
    {
        if (msg.isNoteOn())
        {
            const int note = msg.getNoteNumber();
            heldNotes.erase (std::remove (heldNotes.begin(), heldNotes.end(), note), heldNotes.end());
            const bool legato = ! heldNotes.empty();
            heldNotes.push_back (note);
            midiHeld = true;
            midiNote = note;
            voice.noteOn (note, msg.getVelocity() >= 100, legato);
        }
        else if (msg.isNoteOff())
        {
            heldNotes.erase (std::remove (heldNotes.begin(), heldNotes.end(), msg.getNoteNumber()), heldNotes.end());
            if (heldNotes.empty())
            {
                midiHeld = false;
                midiNote = -1;
                voice.noteOff();
            }
            else if (midiNote == msg.getNoteNumber())
            {
                midiNote = heldNotes.back();
                voice.noteOn (midiNote, false, true);
            }
        }
        else if (msg.isPitchWheel())
        {
            voice.setBendSemitones (((float) msg.getPitchWheelValue() - 8192.0f) / 8192.0f * 2.0f);
        }
        else if (msg.isAllNotesOff() || msg.isAllSoundOff())
        {
            heldNotes.clear();
            midiHeld = false;
            midiNote = -1;
            voice.noteOff();
        }
    };

    auto it = midi.begin();
    for (int i = 0; i < n; ++i)
    {
        while (it != midi.end() && (*it).samplePosition <= i)
        {
            handleMidi ((*it).getMessage());
            ++it;
        }

        if (! midiHeld && seqOn)
        {
            const int intoBefore = sequencer.getSamplesIntoStep();
            Step step;
            const bool trig = sequencer.advance (step);
            if (trig)
            {
                voice.noteOn (step.noteNumber, step.accent, step.slide);
                seqGateOpen = true;
                seqSlide = step.slide;
            }
            if (seqGateOpen && intoBefore == halfGate && ! seqSlide)
            {
                voice.noteOff();
                seqGateOpen = false;
            }
        }

        cvStepper.advance();
        applyCvOverrides (cvStepper.getSmoothed());

        const float s = voice.process();
        left[i] = s;
        if (right) right[i] = s;
    }

    midi.clear();
}

void Lunchbox303AudioProcessor::applyPreset (const SynthParams& p)
{
    auto set = [this] (const char* id, float v)
    {
        if (auto* param = apvts.getParameter (id))
        {
            param->beginChangeGesture();
            if (auto* ranged = dynamic_cast<juce::RangedAudioParameter*> (param))
                param->setValueNotifyingHost (ranged->convertTo0to1 (v));
            else
                param->setValueNotifyingHost (v);
            param->endChangeGesture();
        }
    };
    if (auto* w = apvts.getParameter ("waveform"))
    {
        w->beginChangeGesture();
        w->setValueNotifyingHost (p.waveform);
        w->endChangeGesture();
    }
    set ("tuning", p.tuning);
    set ("cutoff", p.cutoff);
    set ("reso", p.reso);
    set ("envmod", p.envMod);
    set ("decay", p.decay);
    set ("accent", p.accent);
    set ("volume", p.volume);
    set ("wavefold", p.wavefold);
    set ("glide", p.glide);
    set ("tempo", p.tempo);
    set ("sync", p.sync);
    set ("fmvco", p.fmVco);
    set ("fmvcf", p.fmVcf);
    set ("fmenv", p.fmEnv);
    set ("fmdecay", p.fmDecay);
    set ("cv1", p.cv1);
    set ("cv2", p.cv2);
    set ("cv3", p.cv3);
    set ("cv4", p.cv4);
    set ("cvlag", p.cvlag);
    if (auto* mw = apvts.getParameter ("modwave")) { mw->beginChangeGesture(); mw->setValueNotifyingHost (p.modWave); mw->endChangeGesture(); }
    if (auto* mo = apvts.getParameter ("modoct"))  { mo->beginChangeGesture(); mo->setValueNotifyingHost (p.modOct / 4.0f); mo->endChangeGesture(); }
    if (auto* s = apvts.getParameter ("seqon"))
    {
        s->beginChangeGesture();
        s->setValueNotifyingHost (p.seqOn > 0.5f ? 1.0f : 0.0f);
        s->endChangeGesture();
    }
    if (auto* c = apvts.getParameter ("cvon"))
    {
        c->beginChangeGesture();
        c->setValueNotifyingHost (p.cvon > 0.5f ? 1.0f : 0.0f);
        c->endChangeGesture();
    }
    if (auto* d = apvts.getParameter ("cvdiv"))
    {
        d->beginChangeGesture();
        d->setValueNotifyingHost (p.cvdiv / 8.0f);
        d->endChangeGesture();
    }
    for (int i = 1; i <= 4; ++i)
    {
        const auto n = juce::String (i);
        const float dests[] = { p.cvdest1, p.cvdest2, p.cvdest3, p.cvdest4 };
        const float mins[] = { p.cvmin1, p.cvmin2, p.cvmin3, p.cvmin4 };
        const float maxs[] = { p.cvmax1, p.cvmax2, p.cvmax3, p.cvmax4 };
        if (auto* d = apvts.getParameter ("cvdest" + n))
        {
            d->beginChangeGesture();
            d->setValueNotifyingHost (dests[i - 1] / (float) (kNumCvDests - 1));
            d->endChangeGesture();
        }
        set (("cvmin" + n).toRawUTF8(), mins[i - 1]);
        set (("cvmax" + n).toRawUTF8(), maxs[i - 1]);
    }
}

void Lunchbox303AudioProcessor::setCurrentProgram (int index)
{
    const auto& presets = getFactoryPresets();
    if (index < 0 || index >= (int) presets.size()) return;
    currentProgram = index;
    applyPreset (presets[(size_t) index].params);
}

const juce::String Lunchbox303AudioProcessor::getProgramName (int index)
{
    const auto& presets = getFactoryPresets();
    if (index < 0 || index >= (int) presets.size()) return {};
    return presets[(size_t) index].name;
}

void Lunchbox303AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
    {
        xml->setAttribute ("program", currentProgram);
        copyXmlToBinary (*xml, destData);
    }
}

void Lunchbox303AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
    {
        apvts.replaceState (juce::ValueTree::fromXml (*xml));
        currentProgram = xml->getIntAttribute ("program", 0);
    }
}

juce::AudioProcessorEditor* Lunchbox303AudioProcessor::createEditor()
{
    return new Lunchbox303AudioProcessorEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Lunchbox303AudioProcessor();
}
