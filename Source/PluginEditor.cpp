#include "PluginEditor.h"

void Lunchbox303AudioProcessorEditor::styleKnob (juce::Slider& s, const juce::String& name)
{
    s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 64, 16);
    s.setName (name);
    addAndMakeVisible (s);
}

Lunchbox303AudioProcessorEditor::Lunchbox303AudioProcessorEditor (Lunchbox303AudioProcessor& p)
    : AudioProcessorEditor (&p), proc (p)
{
    setSize (640, 360);

    styleKnob (cutoff, "CUT OFF");
    styleKnob (reso, "RESO");
    styleKnob (envMod, "ENV MOD");
    styleKnob (decay, "DECAY");
    styleKnob (accent, "ACCENT");
    styleKnob (volume, "VOLUME");
    styleKnob (tuning, "TUNING");
    styleKnob (tempo, "BPM");

    waveform.addItem ("SAW", 1);
    waveform.addItem ("PULSE", 2);
    addAndMakeVisible (waveform);

    for (int i = 0; i < proc.getNumPrograms(); ++i)
        presetBox.addItem (proc.getProgramName (i), i + 1);
    presetBox.setSelectedItemIndex (proc.getCurrentProgram(), juce::dontSendNotification);
    presetBox.onChange = [this]
    {
        proc.setCurrentProgram (presetBox.getSelectedItemIndex());
    };
    addAndMakeVisible (presetBox);

    addAndMakeVisible (seqOn);
    addAndMakeVisible (hostSync);
    addAndMakeVisible (runButton);
    runButton.onClick = [this]
    {
        auto* param = proc.apvts.getParameter ("seqon");
        if (param == nullptr) return;
        const bool on = param->getValue() < 0.5f;
        param->beginChangeGesture();
        param->setValueNotifyingHost (on ? 1.0f : 0.0f);
        param->endChangeGesture();
    };

    auto& apvts = proc.apvts;
    cutoffA  = std::make_unique<SA> (apvts, "cutoff", cutoff);
    resoA    = std::make_unique<SA> (apvts, "reso", reso);
    envModA  = std::make_unique<SA> (apvts, "envmod", envMod);
    decayA   = std::make_unique<SA> (apvts, "decay", decay);
    accentA  = std::make_unique<SA> (apvts, "accent", accent);
    volumeA  = std::make_unique<SA> (apvts, "volume", volume);
    tuningA  = std::make_unique<SA> (apvts, "tuning", tuning);
    tempoA   = std::make_unique<SA> (apvts, "tempo", tempo);
    seqA     = std::make_unique<BA> (apvts, "seqon", seqOn);
    syncA    = std::make_unique<BA> (apvts, "hostsync", hostSync);
    waveA    = std::make_unique<CA> (apvts, "waveform", waveform);
}

void Lunchbox303AudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff1a1a1e));
    g.setColour (juce::Colour (0xffe8c36a));
    g.setFont (juce::FontOptions (20.0f, juce::Font::bold));
    g.drawText ("LUNCHBOX-303", 12, 8, 300, 28, juce::Justification::centredLeft);
    g.setFont (juce::FontOptions (12.0f));
    g.setColour (juce::Colours::grey);
    g.drawText ("Independent TB-303-style reimplementation (inspired by Eige Kitamura LUNCHBOX-303 v7.2)", 12, 34, getWidth() - 24, 18, juce::Justification::centredLeft);

    auto drawLabel = [&] (juce::Slider& s)
    {
        g.setColour (juce::Colours::lightgrey);
        g.drawText (s.getName(), s.getX(), s.getY() - 16, s.getWidth(), 16, juce::Justification::centred);
    };
    drawLabel (tuning); drawLabel (cutoff); drawLabel (reso); drawLabel (envMod);
    drawLabel (decay); drawLabel (accent); drawLabel (volume); drawLabel (tempo);
}

void Lunchbox303AudioProcessorEditor::resized()
{
    presetBox.setBounds (400, 10, 220, 24);
    waveform.setBounds (12, 60, 100, 24);
    seqOn.setBounds (120, 60, 70, 24);
    hostSync.setBounds (200, 60, 110, 24);
    runButton.setBounds (320, 60, 60, 24);

    const int y = 120;
    const int w = 72;
    const int h = 90;
    int x = 16;
    auto place = [&] (juce::Slider& s)
    {
        s.setBounds (x, y, w, h);
        x += w + 8;
    };
    place (tuning); place (cutoff); place (reso); place (envMod);
    place (decay); place (accent); place (volume); place (tempo);
}
