#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    constexpr float pi = juce::MathConstants<float>::pi;

    float dbToGain (float db) { return juce::Decibels::decibelsToGain(db); }

    struct Preset
    {
        const char* name;
        float hpf, low, mid, high, deess, comp, drive, punch, air, width, out, mix;
    };

    const Preset presets[] =
    {
        { "TRAP BITE",   85.f, -1.f,  3.5f,  5.0f, 0.48f, 0.78f, 0.72f, 0.78f, 0.55f, 0.20f, -1.0f, 1.0f },
        { "RAP FORWARD", 70.f,  0.f,  4.0f,  3.5f, 0.42f, 0.70f, 0.60f, 0.90f, 0.35f, 0.10f, -0.8f, 1.0f },
        { "RADIO AGGRO", 95.f, -2.f,  2.0f,  6.0f, 0.65f, 0.86f, 0.82f, 0.68f, 0.65f, 0.15f, -1.5f, 1.0f },
        { "HYPER POP",   110.f, 1.f,  2.5f,  7.0f, 0.55f, 0.80f, 0.88f, 0.60f, 0.90f, 0.42f, -1.2f, 0.96f },
        { "DARK VOCAL",  65.f,  2.0f, -2.0f,  2.0f, 0.38f, 0.76f, 0.68f, 0.86f, 0.22f, 0.05f, -1.0f, 1.0f },
        { "CLEAN PUNCH", 75.f,  0.f,  1.5f,  2.5f, 0.30f, 0.56f, 0.30f, 0.85f, 0.30f, 0.05f, -0.5f, 1.0f }
    };
}

VoxRageAudioProcessor::VoxRageAudioProcessor()
    : AudioProcessor (BusesProperties()
        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "STATE", createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout VoxRageAudioProcessor::createParameterLayout()
{
    using P = juce::AudioParameterFloat;
    using C = juce::AudioParameterChoice;

    juce::AudioProcessorValueTreeState::ParameterLayout p;

    p.add (std::make_unique<P>("input", "Input", juce::NormalisableRange<float>(-24.f, 18.f, 0.01f), 0.f, "dB"));
    p.add (std::make_unique<P>("hpf", "HPF", juce::NormalisableRange<float>(30.f, 180.f, 1.f), 75.f, "Hz"));
    p.add (std::make_unique<P>("low", "Low", juce::NormalisableRange<float>(-6.f, 6.f, 0.01f), 0.f, "dB"));
    p.add (std::make_unique<P>("mid", "Mid", juce::NormalisableRange<float>(-6.f, 8.f, 0.01f), 2.f, "dB"));
    p.add (std::make_unique<P>("high", "High", juce::NormalisableRange<float>(-6.f, 10.f, 0.01f), 3.f, "dB"));
    p.add (std::make_unique<P>("deess", "De-Ess", juce::NormalisableRange<float>(0.f, 1.f, 0.001f), 0.35f));
    p.add (std::make_unique<P>("comp", "Comp", juce::NormalisableRange<float>(0.f, 1.f, 0.001f), 0.65f));
    p.add (std::make_unique<P>("drive", "Drive", juce::NormalisableRange<float>(0.f, 1.f, 0.001f), 0.45f));
    p.add (std::make_unique<P>("punch", "Punch", juce::NormalisableRange<float>(0.f, 1.f, 0.001f), 0.65f));
    p.add (std::make_unique<P>("air", "Air", juce::NormalisableRange<float>(0.f, 1.f, 0.001f), 0.35f));
    p.add (std::make_unique<P>("width", "Width", juce::NormalisableRange<float>(0.f, 1.f, 0.001f), 0.10f));
    p.add (std::make_unique<P>("output", "Output", juce::NormalisableRange<float>(-18.f, 6.f, 0.01f), -0.5f, "dB"));
    p.add (std::make_unique<P>("mix", "Mix", juce::NormalisableRange<float>(0.f, 1.f, 0.001f), 1.f));

    p.add (std::make_unique<C>("preset", "Preset",
                               juce::StringArray { "TRAP BITE", "RAP FORWARD", "RADIO AGGRO",
                                                   "HYPER POP", "DARK VOCAL", "CLEAN PUNCH" }, 0));
    return p;
}

void VoxRageAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;

    juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) samplesPerBlock, 2 };

    hpfL.prepare(spec); hpfR.prepare(spec);
    lowL.prepare(spec); lowR.prepare(spec);
    midL.prepare(spec); midR.prepare(spec);
    highL.prepare(spec); highR.prepare(spec);

    compressor.prepare(spec);
    limiter.prepare(spec);
    inputGain.prepare(spec);
    outputGain.prepare(spec);

    compressor.setAttack(4.0f);
    compressor.setRelease(55.0f);
    compressor.setRatio(4.0f);
    limiter.setRelease(55.0f);

    updateFilters();
    deEssEnvelope = 0.0f;
    inputPeak = outputPeak = 0.0f;
}

void VoxRageAudioProcessor::releaseResources() {}

bool VoxRageAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto main = layouts.getMainOutputChannelSet();
    return main == juce::AudioChannelSet::mono() || main == juce::AudioChannelSet::stereo();
}

void VoxRageAudioProcessor::updateFilters()
{
    const auto hpf = apvts.getRawParameterValue("hpf")->load();
    const auto low = apvts.getRawParameterValue("low")->load();
    const auto mid = apvts.getRawParameterValue("mid")->load();
    const auto high = apvts.getRawParameterValue("high")->load();
    const auto air = apvts.getRawParameterValue("air")->load();

    *hpfL.state = *Coefficients::makeHighPass(currentSampleRate, hpf, 0.7071);
    *hpfR.state = *Coefficients::makeHighPass(currentSampleRate, hpf, 0.7071);

    *lowL.state = *Coefficients::makePeakFilter(currentSampleRate, 180.0, 0.75, dbToGain(low));
    *lowR.state = *Coefficients::makePeakFilter(currentSampleRate, 180.0, 0.75, dbToGain(low));

    *midL.state = *Coefficients::makePeakFilter(currentSampleRate, 1700.0, 0.9, dbToGain(mid));
    *midR.state = *Coefficients::makePeakFilter(currentSampleRate, 1700.0, 0.9, dbToGain(mid));

    *highL.state = *Coefficients::makeHighShelf(currentSampleRate, 6500.0, 0.7, dbToGain(high + air * 5.0f));
    *highR.state = *Coefficients::makeHighShelf(currentSampleRate, 6500.0, 0.7, dbToGain(high + air * 5.0f));
}

void VoxRageAudioProcessor::applyDrive (juce::AudioBuffer<float>& buffer, float drive)
{
    if (drive <= 0.0001f) return;

    const float amount = 1.0f + drive * 7.0f;
    const float makeup = 1.0f / std::tanh(amount);

    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* data = buffer.getWritePointer(ch);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
            data[i] = std::tanh(data[i] * amount) * makeup;
    }
}

void VoxRageAudioProcessor::applyDeEsser (juce::AudioBuffer<float>& buffer, float amount)
{
    if (amount <= 0.0001f) return;

    const float attack = std::exp(-1.0f / (0.0012f * (float) currentSampleRate));
    const float release = std::exp(-1.0f / (0.075f * (float) currentSampleRate));

    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        float detector = 0.0f;
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            detector += std::abs(buffer.getSample(ch, i));
        detector /= (float) buffer.getNumChannels();

        const float target = juce::jlimit(0.0f, 1.0f, detector * 2.2f);
        deEssEnvelope = target > deEssEnvelope
            ? attack * deEssEnvelope + (1.0f - attack) * target
            : release * deEssEnvelope + (1.0f - release) * target;

        const float sibilance = juce::jlimit(0.0f, 1.0f, (deEssEnvelope - 0.18f) * 3.4f);
        const float gain = 1.0f - sibilance * amount * 0.72f;

        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            buffer.setSample(ch, i, buffer.getSample(ch, i) * gain);
    }
}

void VoxRageAudioProcessor::applyWidth (juce::AudioBuffer<float>& buffer, float width)
{
    if (buffer.getNumChannels() < 2) return;

    const float sideGain = 1.0f + width * 0.75f;
    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        const float l = buffer.getSample(0, i);
        const float r = buffer.getSample(1, i);
        const float mid = 0.5f * (l + r);
        const float side = 0.5f * (l - r) * sideGain;
        buffer.setSample(0, i, mid + side);
        buffer.setSample(1, i, mid - side);
    }
}

void VoxRageAudioProcessor::applyLimiter (juce::AudioBuffer<float>& buffer, float ceilingDb)
{
    limiter.setThreshold(ceilingDb);
    juce::dsp::AudioBlock<float> block(buffer);
    limiter.process(juce::dsp::ProcessContextReplacing<float>(block));
}

void VoxRageAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const auto inputGainDb = apvts.getRawParameterValue("input")->load();
    const auto outputGainDb = apvts.getRawParameterValue("output")->load();
    const auto deess = apvts.getRawParameterValue("deess")->load();
    const auto comp = apvts.getRawParameterValue("comp")->load();
    const auto drive = apvts.getRawParameterValue("drive")->load();
    const auto punch = apvts.getRawParameterValue("punch")->load();
    const auto width = apvts.getRawParameterValue("width")->load();
    const auto mix = apvts.getRawParameterValue("mix")->load();

    updateFilters();

    juce::AudioBuffer<float> dry(buffer.getNumChannels(), buffer.getNumSamples());
    dry.makeCopyOf(buffer);

    const float inGain = dbToGain(inputGainDb);
    buffer.applyGain(inGain);

    inputPeak = buffer.getMagnitude(0, 0, buffer.getNumSamples());
    if (buffer.getNumChannels() > 1)
        inputPeak = juce::jmax(inputPeak, buffer.getMagnitude(1, 0, buffer.getNumSamples()));

    juce::dsp::AudioBlock<float> block(buffer);

    juce::dsp::AudioBlock<float> b0(block);
    juce::dsp::AudioBlock<float> lBlock = b0.getSingleChannelBlock(0);
    juce::dsp::AudioBlock<float> rBlock = b0.getSingleChannelBlock(juce::jmin(1, (int) block.getNumChannels() - 1));

    hpfL.process(juce::dsp::ProcessContextReplacing<float>(lBlock));
    lowL.process(juce::dsp::ProcessContextReplacing<float>(lBlock));
    midL.process(juce::dsp::ProcessContextReplacing<float>(lBlock));
    highL.process(juce::dsp::ProcessContextReplacing<float>(lBlock));

    if (block.getNumChannels() > 1)
    {
        hpfR.process(juce::dsp::ProcessContextReplacing<float>(rBlock));
        lowR.process(juce::dsp::ProcessContextReplacing<float>(rBlock));
        midR.process(juce::dsp::ProcessContextReplacing<float>(rBlock));
        highR.process(juce::dsp::ProcessContextReplacing<float>(rBlock));
    }

    applyDeEsser(buffer, deess);

    compressor.setThreshold(-28.0f + comp * 20.0f);
    compressor.setRatio(2.0f + comp * 8.0f);
    compressor.setAttack(2.0f + (1.0f - punch) * 10.0f);
    compressor.setRelease(30.0f + (1.0f - punch) * 100.0f);
    compressor.setMakeUpGainDecibels(comp * 7.0f);

    compressor.process(juce::dsp::ProcessContextReplacing<float>(block));

    applyDrive(buffer, drive);

    // Punch is parallel dry blend after the main compression/drive stage.
    if (punch > 0.0f)
        buffer.addFrom(0, 0, dry, 0, 0, buffer.getNumSamples(), punch * 0.22f);
    if (buffer.getNumChannels() > 1 && punch > 0.0f)
        buffer.addFrom(1, 0, dry, 1, 0, buffer.getNumSamples(), punch * 0.22f);

    applyWidth(buffer, width);

    applyLimiter(buffer, -0.3f);

    buffer.applyGain(dbToGain(outputGainDb));

    // Final dry/wet.
    const float wet = juce::jlimit(0.0f, 1.0f, mix);
    const float dryGain = 1.0f - wet;
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        buffer.applyGain(ch, 0, buffer.getNumSamples(), wet);
        buffer.addFrom(ch, 0, dry, ch, 0, buffer.getNumSamples(), dryGain);
    }

    outputPeak = buffer.getMagnitude(0, 0, buffer.getNumSamples());
    if (buffer.getNumChannels() > 1)
        outputPeak = juce::jmax(outputPeak, buffer.getMagnitude(1, 0, buffer.getNumSamples()));

    inputMeter.store(juce::jlimit(0.0f, 1.0f, inputPeak));
    outputMeter.store(juce::jlimit(0.0f, 1.0f, outputPeak));
}

void VoxRageAudioProcessor::setCurrentProgram (int index)
{
    index = juce::jlimit(0, getNumPrograms() - 1, index);
    currentProgram = index;
    const auto& p = presets[index];

    auto set = [this](const char* id, float value)
    {
        if (auto* v = apvts.getRawParameterValue(id))
            if (auto* param = dynamic_cast<juce::RangedAudioParameter*>(apvts.getParameter(id)))
                param->setValueNotifyingHost(param->convertTo0to1(value));
    };

    set("hpf", p.hpf);
    set("low", p.low);
    set("mid", p.mid);
    set("high", p.high);
    set("deess", p.deess);
    set("comp", p.comp);
    set("drive", p.drive);
    set("punch", p.punch);
    set("air", p.air);
    set("width", p.width);
    set("output", p.out);
    set("mix", p.mix);
}

const juce::String VoxRageAudioProcessor::getProgramName (int index)
{
    return presets[juce::jlimit(0, getNumPrograms() - 1, index)].name;
}

void VoxRageAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary(*xml, destData);
}

void VoxRageAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
        if (xml->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessorEditor* VoxRageAudioProcessor::createEditor()
{
    return new VoxRageAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new VoxRageAudioProcessor();
}
