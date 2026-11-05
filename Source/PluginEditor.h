#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

class Lunchbox303AudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit Lunchbox303AudioProcessorEditor (Lunchbox303AudioProcessor&);
    ~Lunchbox303AudioProcessorEditor() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    Lunchbox303AudioProcessor& proc;

    juce::Slider cutoff, reso, envMod, decay, accent, volume, tuning, tempo;
    juce::ComboBox waveform, presetBox;
    juce::ToggleButton seqOn { "SEQ" }, hostSync { "HOST SYNC" };
    juce::TextButton runButton { "RUN" };

    using SA = juce::AudioProcessorValueTreeState::SliderAttachment;
    using BA = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using CA = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    std::unique_ptr<SA> cutoffA, resoA, envModA, decayA, accentA, volumeA, tuningA, tempoA;
    std::unique_ptr<BA> seqA, syncA;
    std::unique_ptr<CA> waveA;

    void styleKnob (juce::Slider& s, const juce::String& name);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Lunchbox303AudioProcessorEditor)
};
