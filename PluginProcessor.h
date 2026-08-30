#pragma once
#include <JuceHeader.h>

class ZyrinProcessor : public juce::AudioProcessor {
public:
    ZyrinProcessor();
    ~ZyrinProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorValueTreeState apvts;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "ZYRIN"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override {}
    void setStateInformation(const void*, int) override {}

private:
    juce::AudioBuffer<float> circularBuffer;
    int writePosition = 0;
    
    // Configurable loop length (e.g., 4 beats = 1 bar in 4/4)
    double loopLengthBeats = 4.0;
    
    // Atomic parameters
    std::atomic<float>* loopLengthParam = nullptr;
    std::atomic<float>* modeParam = nullptr;
    std::atomic<float>* smoothParam = nullptr;
    std::atomic<float>* bandLowParam = nullptr;
    std::atomic<float>* bandHighParam = nullptr;
    std::atomic<float>* mixParam = nullptr;
    std::atomic<float>* pitchParam = nullptr;
    std::atomic<float>* grainParam = nullptr;
    std::atomic<float>* bypassParam = nullptr;

    // Filters for crossover
    juce::dsp::LinkwitzRileyFilter<float> lp1[2], hp1[2]; // Cutoff at bandLow
    juce::dsp::LinkwitzRileyFilter<float> lp2[2], hp2[2]; // Cutoff at bandHigh
    juce::dsp::LinkwitzRileyFilter<float> ap1[2];         // Allpass at bandHigh to phase-align low band
    
    float previousSmoothVal = 0.0f; // To track smooth parameter changes if needed, or we just calculate on the fly
    float transportFade = 1.0f; // Smoothes transport start/stop and bypass

    // pitch shifter variables
    juce::AudioBuffer<float> pitchBuffer;
    int pitchWritePos = 0;
    double pitchDelayAccum = 0.0;
    juce::SmoothedValue<float> smoothedGrainSize;

    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ZyrinProcessor)
};