#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

/** Black 303-style knobs with a white pointer line, on a bright orange panel. */
class OrangeLookAndFeel : public juce::LookAndFeel_V4
{
public:
    OrangeLookAndFeel();
    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos,
                           float startAngle, float endAngle, juce::Slider&) override;
    void drawComboBox (juce::Graphics&, int w, int h, bool, int, int, int, int, juce::ComboBox&) override;
    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&, bool, bool) override;
    void drawToggleButton (juce::Graphics&, juce::ToggleButton&, bool, bool) override;
    juce::Font getComboBoxFont (juce::ComboBox&) override { return juce::FontOptions (12.0f, juce::Font::bold); }
};

class Lunchbox303AudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit Lunchbox303AudioProcessorEditor (Lunchbox303AudioProcessor&);
    ~Lunchbox303AudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    Lunchbox303AudioProcessor& proc;
    OrangeLookAndFeel laf;

    juce::Slider cutoff, reso, envMod, decay, accent, volume, tuning, tempo;
    juce::Slider sync, fmVco, fmVcf, fmEnv, fmDecay, wavefold, glide;
    juce::Slider cv1, cv2, cv3, cv4, cvLag;
    juce::Slider cvMin[4], cvMax[4];
    juce::ComboBox waveform, presetBox, modWave, modOct, cvDiv;
    juce::ComboBox cvDest[4];
    juce::ToggleButton seqOn { "SEQ" }, hostSync { "HOST SYNC" }, cvOn { "CV" };
    juce::TextButton runButton { "RUN" };

    using SA = juce::AudioProcessorValueTreeState::SliderAttachment;
    using BA = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using CA = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    std::unique_ptr<SA> cutoffA, resoA, envModA, decayA, accentA, volumeA, tuningA, tempoA;
    std::unique_ptr<SA> syncA2, fmVcoA, fmVcfA, fmEnvA, fmDecayA, wavefoldA, glideA;
    std::unique_ptr<SA> cv1A, cv2A, cv3A, cv4A, cvLagA;
    std::unique_ptr<SA> cvMinA[4], cvMaxA[4];
    std::unique_ptr<BA> seqA, syncA, cvOnA;
    std::unique_ptr<CA> waveA, modWaveA, modOctA, cvDivA;
    std::unique_ptr<CA> cvDestA[4];

    juce::Rectangle<int> modBox, cvBox;
    struct Label { juce::String text; juce::Component* c; };
    std::vector<Label> labels;

    void styleKnob (juce::Slider& s, const juce::String& name, bool bipolar = false);
    void styleSmallSlider (juce::Slider& s, const juce::String& name);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Lunchbox303AudioProcessorEditor)
};
