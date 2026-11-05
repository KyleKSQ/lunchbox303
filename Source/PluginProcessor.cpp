#include "PluginProcessor.h"
#include <algorithm>
#include "PluginEditor.h"

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
    return { params.begin(), params.end() };
}

void Lunchbox303AudioProcessor::prepareToPlay (double sampleRate, int)
{
    voice.prepare (sampleRate);
    sequencer.prepare (sampleRate);
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
    voice.setModWaveTriangle ((int) apvts.getRawParameterValue ("modwave")->load() == 1);
    voice.setModOctave ((int) apvts.getRawParameterValue ("modoct")->load() - 2);
    voice.setModSync (apvts.getRawParameterValue ("sync")->load());
    voice.setFmVco (apvts.getRawParameterValue ("fmvco")->load());
    voice.setFmVcf (apvts.getRawParameterValue ("fmvcf")->load());
    voice.setFmEnvAmt (apvts.getRawParameterValue ("fmenv")->load());
    voice.setFmDecay (apvts.getRawParameterValue ("fmdecay")->load());

    const bool hostSync = apvts.getRawParameterValue ("hostsync")->load() > 0.5f;
    if (hostSync)
    {
        if (auto* playHead = getPlayHead())
        {
            if (auto pos = playHead->getPosition())
            {
                if (auto bpm = pos->getBpm())
                    sequencer.setTempo (*bpm);
            }
        }
    }
    else
    {
        sequencer.setTempo (apvts.getRawParameterValue ("tempo")->load());
    }
}

void Lunchbox303AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();
    applyParamsToVoice();

    const bool seqOn = apvts.getRawParameterValue ("seqon")->load() > 0.5f;
    const bool hostSync = apvts.getRawParameterValue ("hostsync")->load() > 0.5f;

    if (hostSync)
    {
        bool playing = false;
        if (auto* playHead = getPlayHead())
            if (auto pos = playHead->getPosition())
                playing = pos->getIsPlaying();
        sequencer.setRunning (seqOn && playing);
    }
    else
    {
        sequencer.setRunning (seqOn);
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
    set ("tempo", p.tempo);
    set ("sync", p.sync);
    set ("fmvco", p.fmVco);
    set ("fmvcf", p.fmVcf);
    set ("fmenv", p.fmEnv);
    set ("fmdecay", p.fmDecay);
    if (auto* mw = apvts.getParameter ("modwave")) { mw->beginChangeGesture(); mw->setValueNotifyingHost (p.modWave); mw->endChangeGesture(); }
    if (auto* mo = apvts.getParameter ("modoct"))  { mo->beginChangeGesture(); mo->setValueNotifyingHost (p.modOct / 4.0f); mo->endChangeGesture(); }
    if (auto* s = apvts.getParameter ("seqon"))
    {
        s->beginChangeGesture();
        s->setValueNotifyingHost (p.seqOn > 0.5f ? 1.0f : 0.0f);
        s->endChangeGesture();
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
