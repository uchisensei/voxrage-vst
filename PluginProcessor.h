#pragma once
#include <JuceHeader.h>

class VoxRageAudioProcessor final : public juce::AudioProcessor
{
public:
    VoxRageAudioProcessor();
    ~VoxRageAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "VoxRage"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 6; }
    int getCurrentProgram() override { return currentProgram; }
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }
    float getInputMeter() const noexcept { return inputMeter.load(); }
    float getOutputMeter() const noexcept { return outputMeter.load(); }

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
    using Filter = juce::dsp::IIR::Filter<float>;
    using Coefficients = juce::dsp::IIR::Coefficients<float>;

    void updateFilters();
    void applyDeEsser(juce::AudioBuffer<float>& buffer, float amount);
    void applyDrive(juce::AudioBuffer<float>& buffer, float drive);
    void applyWidth(juce::AudioBuffer<float>& buffer, float width);
    void applyLimiter(juce::AudioBuffer<float>& buffer, float ceilingDb);

    juce::AudioProcessorValueTreeState apvts;

    Filter hpfL, hpfR;
    Filter lowL, lowR, midL, midR, highL, highR;

    juce::dsp::Compressor<float> compressor;
    juce::dsp::Limiter<float> limiter;

    juce::dsp::Gain<float> inputGain;
    juce::dsp::Gain<float> outputGain;

    double currentSampleRate = 44100.0;
    int currentProgram = 0;

    float deEssEnvelope = 0.0f;
    float inputPeak = 0.0f;
    float outputPeak = 0.0f;

    std::atomic<float> inputMeter { 0.0f };
    std::atomic<float> outputMeter { 0.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VoxRageAudioProcessor)
};
