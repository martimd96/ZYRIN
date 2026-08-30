#pragma once
#include <JuceHeader.h>

class ZyrinProcessor : public juce::AudioProcessor {
public:
    ZyrinProcessor();
    ~ZyrinProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ZyrinProcessor)
};