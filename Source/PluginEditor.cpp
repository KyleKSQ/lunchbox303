#include "PluginEditor.h"

namespace
{
    const juce::Colour kOrange   (0xffff6a00);
    const juce::Colour kOrangeHi (0xffff8a2a);
    const juce::Colour kOrangeLo (0xffe85600);
    const juce::Colour kInk      (0xff141414);
    const juce::Colour kWhite    (0xfffff6ec);
}

OrangeLookAndFeel::OrangeLookAndFeel()
{
    setColour (juce::ComboBox::backgroundColourId, kInk);
    setColour (juce::ComboBox::textColourId, kWhite);
    setColour (juce::ComboBox::outlineColourId, kInk);
    setColour (juce::ComboBox::arrowColourId, kOrange);
    setColour (juce::PopupMenu::backgroundColourId, kInk);
    setColour (juce::PopupMenu::textColourId, kWhite);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, kOrange);
    setColour (juce::PopupMenu::highlightedTextColourId, kInk);
    setColour (juce::Slider::textBoxTextColourId, kInk);
    setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    setColour (juce::TextButton::buttonColourId, kInk);
    setColour (juce::TextButton::textColourOffId, kWhite);
}

void OrangeLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos,
                                          float startAngle, float endAngle, juce::Slider&)
{
    const float size = (float) std::min (w, h);
    const auto c = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h).getCentre();
    const float r = size * 0.5f - 6.0f;
    const float angle = startAngle + pos * (endAngle - startAngle);

    // printed scale ticks
    g.setColour (kInk);
    for (int i = 0; i <= 10; ++i)
    {
        const float a = startAngle + (float) i / 10.0f * (endAngle - startAngle);
        const float r1 = r + 2.0f, r2 = r + (i % 5 == 0 ? 6.0f : 4.0f);
        g.drawLine (c.x + std::sin (a) * r1, c.y - std::cos (a) * r1,
                    c.x + std::sin (a) * r2, c.y - std::cos (a) * r2, i % 5 == 0 ? 1.6f : 1.0f);
    }

    // knob body
    g.setColour (juce::Colours::black.withAlpha (0.35f));
    g.fillEllipse (c.x - r + 1.5f, c.y - r + 2.5f, r * 2, r * 2);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff3a3a3a), c.x - r * 0.5f, c.y - r * 0.6f,
                                             juce::Colour (0xff0c0c0c), c.x + r * 0.6f, c.y + r * 0.8f, false));
    g.fillEllipse (c.x - r, c.y - r, r * 2, r * 2);
    g.setColour (juce::Colour (0xff050505));
    g.drawEllipse (c.x - r, c.y - r, r * 2, r * 2, 1.2f);
    const float rc = r * 0.62f;
    g.setColour (juce::Colour (0xff1c1c1c));
    g.fillEllipse (c.x - rc, c.y - rc, rc * 2, rc * 2);

    // white pointer
    juce::Path p;
    p.addRoundedRectangle (-1.8f, -r + 1.0f, 3.6f, r * 0.55f, 1.2f);
    g.setColour (juce::Colours::white);
    g.fillPath (p, juce::AffineTransform::rotation (angle).translated (c.x, c.y));
}

void OrangeLookAndFeel::drawComboBox (juce::Graphics& g, int w, int h, bool, int, int, int, int, juce::ComboBox&)
{
    g.setColour (kInk);
    g.fillRoundedRectangle (0.0f, 0.0f, (float) w, (float) h, 3.0f);
    g.setColour (kOrange);
    g.drawRoundedRectangle (0.5f, 0.5f, (float) w - 1.0f, (float) h - 1.0f, 3.0f, 1.2f);
    juce::Path arrow;
    const float ax = (float) w - 14.0f, ay = (float) h * 0.5f;
    arrow.addTriangle (ax, ay - 3.0f, ax + 8.0f, ay - 3.0f, ax + 4.0f, ay + 3.0f);
    g.setColour (kOrange);
    g.fillPath (arrow);
}

void OrangeLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour&, bool over, bool down)
{
    auto r = b.getLocalBounds().toFloat().reduced (0.5f);
    g.setColour (down ? kOrangeLo : (over ? juce::Colour (0xff2a2a2a) : kInk));
    g.fillRoundedRectangle (r, 3.0f);
    g.setColour (kOrange);
    g.drawRoundedRectangle (r, 3.0f, 1.2f);
}

void OrangeLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& b, bool, bool)
{
    auto r = b.getLocalBounds().toFloat().reduced (0.5f);
    const bool on = b.getToggleState();
    g.setColour (on ? juce::Colour (0xffffe14a) : kInk);
    g.fillRoundedRectangle (r, 3.0f);
    g.setColour (kOrange);
    g.drawRoundedRectangle (r, 3.0f, 1.2f);
    g.setColour (on ? kInk : kWhite);
    g.setFont (juce::FontOptions (12.0f, juce::Font::bold));
    g.drawText (b.getButtonText(), b.getLocalBounds(), juce::Justification::centred);
}

void Lunchbox303AudioProcessorEditor::styleKnob (juce::Slider& s, const juce::String& name)
{
    s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    s.setRotaryParameters (juce::degreesToRadians (225.0f), juce::degreesToRadians (495.0f), true);
    s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 62, 14);
    s.setName (name);
    addAndMakeVisible (s);
    labels.push_back ({ name, &s });
}

Lunchbox303AudioProcessorEditor::Lunchbox303AudioProcessorEditor (Lunchbox303AudioProcessor& p)
    : AudioProcessorEditor (&p), proc (p)
{
    setLookAndFeel (&laf);
    setSize (760, 470);

    styleKnob (tuning, "TUNING");
    styleKnob (cutoff, "CUT OFF");
    styleKnob (reso, "RESONANCE");
    styleKnob (envMod, "ENV MOD");
    styleKnob (decay, "DECAY");
    styleKnob (accent, "ACCENT");
    styleKnob (volume, "VOLUME");

    styleKnob (sync, "SYNC");
    styleKnob (fmVco, "VCO FM");
    styleKnob (fmVcf, "VCF FM");
    styleKnob (fmEnv, "FM ENV");
    styleKnob (fmDecay, "FM DECAY");

    styleKnob (tempo, "BPM");
    labels.pop_back(); // BPM label is drawn on the dark strip instead
    tempo.setColour (juce::Slider::textBoxTextColourId, kWhite);

    waveform.addItem ("SAW", 1);
    waveform.addItem ("PULSE", 2);
    addAndMakeVisible (waveform);

    modWave.addItem ("SINE", 1);
    modWave.addItem ("TRI", 2);
    addAndMakeVisible (modWave);
    for (int i = 0; i < 5; ++i)
        modOct.addItem (juce::StringArray { "-2 OCT", "-1 OCT", "0 OCT", "+1 OCT", "+2 OCT" }[i], i + 1);
    addAndMakeVisible (modOct);

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
    syncA2   = std::make_unique<SA> (apvts, "sync", sync);
    fmVcoA   = std::make_unique<SA> (apvts, "fmvco", fmVco);
    fmVcfA   = std::make_unique<SA> (apvts, "fmvcf", fmVcf);
    fmEnvA   = std::make_unique<SA> (apvts, "fmenv", fmEnv);
    fmDecayA = std::make_unique<SA> (apvts, "fmdecay", fmDecay);
    seqA     = std::make_unique<BA> (apvts, "seqon", seqOn);
    syncA    = std::make_unique<BA> (apvts, "hostsync", hostSync);
    waveA    = std::make_unique<CA> (apvts, "waveform", waveform);
    modWaveA = std::make_unique<CA> (apvts, "modwave", modWave);
    modOctA  = std::make_unique<CA> (apvts, "modoct", modOct);
}

Lunchbox303AudioProcessorEditor::~Lunchbox303AudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

void Lunchbox303AudioProcessorEditor::paint (juce::Graphics& g)
{
    const int W = getWidth(), H = getHeight();

    // orange panel
    g.setGradientFill (juce::ColourGradient (kOrangeHi, 0.0f, 0.0f, kOrange, 0.0f, (float) H * 0.5f, false));
    g.fillAll();
    g.setColour (kInk);
    g.drawRect (0, 0, W, H, 3);

    // header
    g.setColour (kInk);
    g.setFont (juce::FontOptions (26.0f, juce::Font::bold));
    g.drawText ("LUNCHBOX-303", 20, 10, 300, 30, juce::Justification::centredLeft);
    g.setFont (juce::FontOptions (12.5f, juce::Font::bold));
    g.drawText ("COMPUTER CONTROLLED BASS LINE", 22, 38, 300, 16, juce::Justification::centredLeft);

    // edition badge
    juce::Rectangle<float> badge (W - 250.0f, 14.0f, 230.0f, 38.0f);
    g.setColour (kInk);
    g.fillRoundedRectangle (badge, 4.0f);
    g.setColour (kOrange);
    g.setFont (juce::FontOptions (20.0f, juce::Font::bold));
    g.drawText ("KLSTRFK edition", badge, juce::Justification::centred);

    // thin silkscreen divider lines
    g.setColour (kInk);
    g.fillRect (20, 62, W - 40, 2);

    // modulator section
    g.setColour (kInk);
    g.drawRoundedRectangle (modBox.toFloat(), 6.0f, 2.0f);
    g.setFont (juce::FontOptions (12.5f, juce::Font::bold));
    const juce::String title = "MODULATOR OSC 2  (FM SOURCE ONLY, NOT IN AUDIO OUT)";
    const int tw = (int) (title.length() * 7.4f) + 14;
    g.setColour (kOrange);
    g.fillRect (modBox.getX() + 14, modBox.getY() - 9, tw, 18);
    g.setColour (kInk);
    g.drawText (title, modBox.getX() + 14, modBox.getY() - 9, tw, 18, juce::Justification::centred);

    // bottom strip (dark, like the 303's lower control area)
    g.setColour (kInk);
    g.fillRect (0, H - 78, W, 78);

    // knob labels
    g.setFont (juce::FontOptions (12.0f, juce::Font::bold));
    g.setColour (kInk);
    for (auto& l : labels)
        g.drawText (l.text, l.c->getX() - 6, l.c->getY() - 14, l.c->getWidth() + 12, 14, juce::Justification::centred);

    g.setFont (juce::FontOptions (11.0f, juce::Font::bold));
    g.setColour (kWhite);
    g.drawText ("OSC 1 WAVE", waveform.getX(), waveform.getY() - 14, waveform.getWidth(), 14, juce::Justification::centred);
    g.setColour (kInk);
    g.drawText ("WAVE", modWave.getX(), modWave.getY() - 14, modWave.getWidth(), 14, juce::Justification::centred);
    g.drawText ("OCTAVE", modOct.getX(), modOct.getY() - 14, modOct.getWidth(), 14, juce::Justification::centred);
    g.setColour (kWhite);
    g.drawText ("PRESET", presetBox.getX(), presetBox.getY() - 14, presetBox.getWidth(), 14, juce::Justification::centred);

    g.setColour (kWhite);
    g.drawText ("BPM", tempo.getX() - 34, tempo.getY(), 32, tempo.getHeight(), juce::Justification::centredRight);
}

void Lunchbox303AudioProcessorEditor::resized()
{
    const int W = getWidth(), H = getHeight();

    // Row 1: classic 303 knobs
    const int kw = 76, kh = 92, gap = (W - 40 - 7 * kw) / 6;
    int x = 20;
    const int y1 = 100;
    for (auto* s : { &tuning, &cutoff, &reso, &envMod, &decay, &accent, &volume })
    {
        s->setBounds (x, y1, kw, kh);
        x += kw + gap;
    }

    // Modulator box
    modBox = { 20, 232, W - 40, 150 };
    const int my = modBox.getY() + 40;

    const int cw = 88, cg = (modBox.getWidth() - 24 - 7 * cw) / 6;
    int mx = modBox.getX() + 12;
    modOct.setBounds  (mx, my + 6, cw, 24);  mx += cw + cg;
    modWave.setBounds (mx, my + 6, cw, 24);  mx += cw + cg;
    sync.setBounds    (mx, my - 12, cw, 92); mx += cw + cg;
    fmVco.setBounds   (mx, my - 12, cw, 92); mx += cw + cg;
    fmVcf.setBounds   (mx, my - 12, cw, 92); mx += cw + cg;
    fmEnv.setBounds   (mx, my - 12, cw, 92); mx += cw + cg;
    fmDecay.setBounds (mx, my - 12, cw, 92);

    // Bottom strip
    const int by = H - 78;
    waveform.setBounds (20, by + 38, 90, 26);
    presetBox.setBounds (130, by + 38, 170, 26);
    seqOn.setBounds (320, by + 38, 60, 26);
    runButton.setBounds (388, by + 38, 60, 26);
    hostSync.setBounds (456, by + 38, 96, 26);
    tempo.setTextBoxStyle (juce::Slider::TextBoxRight, false, 54, 16);
    tempo.setBounds (W - 20 - 100, by + 20, 100, 52);
}
