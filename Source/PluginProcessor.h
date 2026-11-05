#pragma once
#include <vector>
#include <juce_audio_processors/juce_audio_processors.h>
#include "VoiceEngine.h"
#include "Sequencer.h"
#include "Presets.h"

class Lunchbox303AudioProcessor : public juce::AudioProcessor
{
public:
    Lunchbox303AudioProcessor();
    ~Lunchbox303AudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Lunchbox303"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.5; }

    int getNumPrograms() override { return (int) getFactoryPresets().size(); }
    int getCurrentProgram() override { return currentProgram; }
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;
    Sequencer& getSequencer() noexcept { return sequencer; }

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
    void applyParamsToVoice();
    void applyPreset (const SynthParams& p);

    VoiceEngine voice;
    Sequencer sequencer;
    int currentProgram { 0 };
    std::vector<int> heldNotes;
    bool midiHeld { false };
    int midiNote { -1 };
    bool seqGateOpen { false };
    bool seqSlide { false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Lunchbox303AudioProcessor)
};
