#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class ZyrinEditor : public juce::AudioProcessorEditor {
public:
    ZyrinEditor (ZyrinProcessor&);
    ~ZyrinEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    ZyrinProcessor& audioProcessor;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ZyrinEditor)
};