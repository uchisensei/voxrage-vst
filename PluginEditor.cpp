#include "PluginEditor.h"

namespace
{
    juce::Colour purple = juce::Colour(0xffa45cff);
    juce::Colour purple2 = juce::Colour(0xff6b2cff);
    juce::Colour bg = juce::Colour(0xff09070d);
    juce::Colour panel = juce::Colour(0xff15101c);

    void drawGlow(juce::Graphics& g, juce::Rectangle<float> r, juce::Colour c)
    {
        for (int i = 5; i >= 1; --i)
        {
            auto rr = r.expanded((float) i * 3.0f);
            g.setColour(c.withAlpha(0.018f * (float) (6 - i)));
            g.fillRoundedRectangle(rr, 18.0f);
        }
    }
}

VoxRageLookAndFeel::VoxRageLookAndFeel()
{
    setColour(juce::Slider::rotarySliderFillColourId, purple);
    setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff30223b));
    setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff110d17));
    setColour(juce::ComboBox::textColourId, juce::Colours::white);
}

void VoxRageLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h,
                                          float sliderPos, float start, float end, juce::Slider&)
{
    const float cx = x + w * 0.5f;
    const float cy = y + h * 0.42f;
    const float radius = juce::jmin(w, h) * 0.31f;

    g.setColour(juce::Colour(0xff0b0910));
    g.fillEllipse(cx - radius, cy - radius, radius * 2.0f, radius * 2.0f);

    g.setColour(juce::Colour(0xff29202f));
    g.drawEllipse(cx - radius, cy - radius, radius * 2.0f, radius * 2.0f, 2.0f);

    juce::Path arc;
    arc.addCentredArc(cx, cy, radius + 7.0f, radius + 7.0f, 0.0f,
                      start, start + (end - start) * sliderPos, true);
    g.setColour(purple);
    g.strokePath(arc, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    const float angle = start + (end - start) * sliderPos;
    const float px = cx + std::cos(angle) * (radius - 4.0f);
    const float py = cy + std::sin(angle) * (radius - 4.0f);
    g.setColour(juce::Colours::white);
    g.fillEllipse(px - 3.0f, py - 3.0f, 6.0f, 6.0f);
}

void VoxRageLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& b,
                                           bool highlighted, bool)
{
    auto r = b.getLocalBounds().toFloat().reduced(3.0f);
    g.setColour(b.getToggleState() ? purple2 : juce::Colour(0xff211a28));
    g.fillRoundedRectangle(r, 10.0f);
    g.setColour(b.getToggleState() ? purple : juce::Colour(0xff55435f));
    g.drawRoundedRectangle(r, 10.0f, 1.5f);
    g.setColour(juce::Colours::white);
    g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    g.drawText(b.getButtonText(), r, juce::Justification::centred);
}

VoxRageAudioProcessorEditor::VoxRageAudioProcessorEditor(VoxRageAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    setLookAndFeel(&lf);
    setResizable(true, true);
    setSize(1080, 690);
    setResizeLimits(820, 540, 1600, 1000);

    auto addKnob = [this](Knob& k, const juce::String& name, const juce::String& suffix)
    {
        configureKnob(k, name, suffix);
        addAndMakeVisible(k);
    };

    addKnob(input, "INPUT", " dB");
    addKnob(hpf, "HPF", " Hz");
    addKnob(low, "LOW", " dB");
    addKnob(mid, "MID", " dB");
    addKnob(high, "HIGH", " dB");
    addKnob(deess, "DE-ESS", "");
    addKnob(comp, "COMP", "");
    addKnob(drive, "DRIVE", "");
    addKnob(punch, "PUNCH", "");
    addKnob(air, "AIR", "");
    addKnob(width, "WIDTH", "");
    addKnob(output, "OUT", " dB");
    addKnob(mix, "MIX", "");

    inputA = std::make_unique<Attachment>(processor.getAPVTS(), "input", input);
    hpfA = std::make_unique<Attachment>(processor.getAPVTS(), "hpf", hpf);
    lowA = std::make_unique<Attachment>(processor.getAPVTS(), "low", low);
    midA = std::make_unique<Attachment>(processor.getAPVTS(), "mid", mid);
    highA = std::make_unique<Attachment>(processor.getAPVTS(), "high", high);
    deessA = std::make_unique<Attachment>(processor.getAPVTS(), "deess", deess);
    compA = std::make_unique<Attachment>(processor.getAPVTS(), "comp", comp);
    driveA = std::make_unique<Attachment>(processor.getAPVTS(), "drive", drive);
    punchA = std::make_unique<Attachment>(processor.getAPVTS(), "punch", punch);
    airA = std::make_unique<Attachment>(processor.getAPVTS(), "air", air);
    widthA = std::make_unique<Attachment>(processor.getAPVTS(), "width", width);
    outputA = std::make_unique<Attachment>(processor.getAPVTS(), "output", output);
    mixA = std::make_unique<Attachment>(processor.getAPVTS(), "mix", mix);

    addAndMakeVisible(presetBox);
    presetBox.addItem("TRAP BITE", 1);
    presetBox.addItem("RAP FORWARD", 2);
    presetBox.addItem("RADIO AGGRO", 3);
    presetBox.addItem("HYPER POP", 4);
    presetBox.addItem("DARK VOCAL", 5);
    presetBox.addItem("CLEAN PUNCH", 6);
    presetBox.setSelectedId(processor.getCurrentProgram() + 1, juce::dontSendNotification);
    presetBox.onChange = [this] { loadPreset(presetBox.getSelectedId() - 1); };

    presetLabel.setText("PRESETS", juce::dontSendNotification);
    presetLabel.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    presetLabel.setColour(juce::Label::textColourId, purple);
    addAndMakeVisible(presetLabel);

    powerButton.setClickingTogglesState(true);
    powerButton.setToggleState(true, juce::dontSendNotification);
    addAndMakeVisible(powerButton);

    title.setText("VØX//RAGE", juce::dontSendNotification);
    title.setFont(juce::FontOptions(28.0f, juce::Font::bold));
    title.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(title);

    status.setText("AGGRESSIVE VOCAL PROCESSOR  //  READY", juce::dontSendNotification);
    status.setFont(juce::FontOptions(10.0f));
    status.setColour(juce::Label::textColourId, purple);
    addAndMakeVisible(status);

    presetA = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(processor.getAPVTS(), "preset", presetBox);

    startTimerHz(30);
}

void VoxRageAudioProcessorEditor::configureKnob(Knob& k, const juce::String& name, const juce::String& suffix)
{
    k.setName(name);
    k.setTextValueSuffix(suffix);
    k.setPopupDisplayEnabled(true, false, this);
    k.setDoubleClickReturnValue(true, 0.0);
    k.setNumDecimalPlacesToDisplay(1);
}

void VoxRageAudioProcessorEditor::loadPreset(int index)
{
    processor.setCurrentProgram(index);
    presetBox.setSelectedId(index + 1, juce::dontSendNotification);
    status.setText("NOW MIXING:  " + processor.getProgramName(index), juce::dontSendNotification);
}

void VoxRageAudioProcessorEditor::timerCallback()
{
    visualPhase += 0.11f;
    repaint();
}

void VoxRageAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(bg);

    auto bounds = getLocalBounds().toFloat();
    juce::ColourGradient gradient(juce::Colour(0xff100a18), 0, 0,
                                  juce::Colour(0xff06050a), bounds.getWidth(), bounds.getHeight(), false);
    g.setGradientFill(gradient);
    g.fillAll();

    // Ambient glows.
    g.setColour(purple.withAlpha(0.035f));
    g.fillEllipse(bounds.getCentreX() - 330, bounds.getCentreY() - 250, 660, 500);
    g.setColour(purple2.withAlpha(0.025f));
    g.fillEllipse(bounds.getRight() - 430, bounds.getBottom() - 280, 520, 420);

    auto header = juce::Rectangle<float>(24, 18, bounds.getWidth() - 48, 62);
    g.setColour(juce::Colour(0xcc0e0a13));
    g.fillRoundedRectangle(header, 18.0f);
    g.setColour(juce::Colour(0xff2d2037));
    g.drawRoundedRectangle(header, 18.0f, 1.0f);

    g.setColour(purple);
    g.setFont(juce::FontOptions(12.0f, juce::Font::bold));
    g.drawText("VOCAL", 160, 37, 70, 20, juce::Justification::centred);
    g.setColour(juce::Colours::white.withAlpha(0.65f));
    g.drawText("TONE", 250, 37, 70, 20, juce::Justification::centred);
    g.drawText("DYNAMICS", 335, 37, 100, 20, juce::Justification::centred);
    g.drawText("SPACE", 455, 37, 70, 20, juce::Justification::centred);

    auto display = displayArea.toFloat();
    drawGlow(g, display, purple);
    g.setColour(panel);
    g.fillRoundedRectangle(display, 22.0f);
    g.setColour(juce::Colour(0xff3c284a));
    g.drawRoundedRectangle(display, 22.0f, 1.0f);

    auto inner = display.reduced(20.0f);
    g.setColour(juce::Colour(0xff0c0910));
    g.fillRoundedRectangle(inner, 16.0f);

    // Analyzer grid.
    g.setColour(purple.withAlpha(0.06f));
    for (int i = 1; i < 8; ++i)
        g.drawHorizontalLine((int) (inner.getY() + inner.getHeight() * i / 8.0f), inner.getX(), inner.getRight());
    for (int i = 1; i < 12; ++i)
        g.drawVerticalLine((int) (inner.getX() + inner.getWidth() * i / 12.0f), inner.getY(), inner.getBottom());

    // Animated vocal-energy bars.
    const int bars = 64;
    const float meter = processor.getOutputMeter();
    for (int i = 0; i < bars; ++i)
    {
        const float x = inner.getX() + i * inner.getWidth() / (float) bars;
        const float wobble = 0.25f + 0.75f * std::abs(std::sin(visualPhase * 1.4f + i * 0.37f));
        const float h = juce::jlimit(3.0f, inner.getHeight() * 0.72f,
                                     inner.getHeight() * (0.05f + meter * wobble * 0.82f));
        g.setColour(purple.withAlpha(0.25f + 0.5f * wobble));
        g.fillRoundedRectangle(x, inner.getCentreY() - h * 0.5f, 2.4f, h, 1.2f);
    }

    g.setColour(juce::Colours::white.withAlpha(0.92f));
    g.setFont(juce::FontOptions(17.0f, juce::Font::bold));
    g.drawText("NOW MIXING:  " + processor.getProgramName(processor.getCurrentProgram()),
               inner.getX() + 18, inner.getY() + 18, inner.getWidth() - 36, 26,
               juce::Justification::centred);

    g.setColour(purple.withAlpha(0.8f));
    g.setFont(juce::FontOptions(10.0f, juce::Font::bold));
    g.drawText("PUSH IT.  SHAPE IT.  OWN THE FRONT.", inner.getX() + 18, inner.getBottom() - 30,
               inner.getWidth() - 36, 20, juce::Justification::centred);

    // Meter rails.
    auto drawMeter = [&g](juce::Rectangle<float> r, float value, const char* text)
    {
        g.setColour(juce::Colour(0xff21192a));
        g.fillRoundedRectangle(r, 5.0f);
        auto fill = r.withHeight(r.getHeight() * juce::jlimit(0.0f, 1.0f, value));
        fill.setY(r.getBottom() - fill.getHeight());
        g.setColour(purple);
        g.fillRoundedRectangle(fill, 5.0f);
        g.setColour(juce::Colours::white.withAlpha(0.5f));
        g.setFont(juce::FontOptions(9.0f));
        g.drawText(text, r.getX() - 12, r.getY() - 16, r.getWidth() + 24, 14, juce::Justification::centred);
    };

    drawMeter({display.getRight() - 20, display.getY() + 30, 7, display.getHeight() - 60}, processor.getOutputMeter(), "OUT");
    drawMeter({display.getX() + 13, display.getY() + 30, 7, display.getHeight() - 60}, processor.getInputMeter(), "IN");
}

void VoxRageAudioProcessorEditor::resized()
{
    auto b = getLocalBounds().reduced(24);
    title.setBounds(b.getX(), b.getY() + 13, 150, 34);
    status.setBounds(b.getX() + 160, b.getY() + 19, 350, 24);

    presetLabel.setBounds(b.getRight() - 300, b.getY() + 8, 70, 20);
    presetBox.setBounds(b.getRight() - 220, b.getY() + 5, 155, 28);
    powerButton.setBounds(b.getRight() - 52, b.getY() + 4, 48, 30);

    displayArea = { b.getX() + 10, b.getY() + 78, b.getWidth() - 20, (int) (b.getHeight() * 0.43f) };

    const int gap = 8;
    const int rowsTop = displayArea.getBottom() + 12;
    const int rowH = juce::jmax(80, (b.getBottom() - rowsTop - gap) / 2);

    const int columns = 7;
    const int w = (b.getWidth() - gap * (columns - 1)) / columns;
    Knob* top[] = { &hpf, &low, &mid, &high, &deess, &comp, &drive };
    for (int i = 0; i < columns; ++i)
        top[i]->setBounds(b.getX() + i * (w + gap), rowsTop, w, rowH);

    const int row2Y = rowsTop + rowH + gap;
    const int rowW = (b.getWidth() - gap * 5) / 6;
    Knob* bottom[] = { &input, &punch, &air, &width, &output, &mix };
    for (int i = 0; i < 6; ++i)
        bottom[i]->setBounds(b.getX() + i * (rowW + gap), row2Y, rowW, rowH);
}
