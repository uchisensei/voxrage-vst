#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class VoxRageLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    VoxRageLookAndFeel();

    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                           juce::Slider&) override;
    void drawToggleButton (juce::Graphics&, juce::ToggleButton&, bool, bool) override;
};

class VoxRageAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                           private juce::Timer
{
public:
    explicit VoxRageAudioProcessorEditor (VoxRageAudioProcessor&);
    ~VoxRageAudioProcessorEditor() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    struct Knob : juce::Slider
    {
        Knob() {
            setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
            setTextBoxStyle(juce::Slider::TextBoxBelow, false, 74, 22);
            setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
            setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff100b18));
            setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0xff5b347b));
        }
    };

    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    VoxRageAudioProcessor& processor;
    VoxRageLookAndFeel lf;

    Knob input, hpf, low, mid, high, deess, comp, drive, punch, air, width, output, mix;
    juce::ComboBox presetBox;
    juce::TextButton powerButton {"POWER"};
    juce::Label title, status, presetLabel;
    juce::Rectangle<int> displayArea;

    std::unique_ptr<Attachment> inputA, hpfA, lowA, midA, highA, deessA, compA, driveA;
    std::unique_ptr<Attachment> punchA, airA, widthA, outputA, mixA;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> presetA;

    float visualPhase = 0.0f;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VoxRageAudioProcessorEditor)

    void timerCallback() override;
    void configureKnob(Knob&, const juce::String&, const juce::String&);
    void loadPreset(int index);
};
