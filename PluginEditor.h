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
    
    juce::ComboBox loopLengthBox;
    juce::ComboBox modeBox;
    juce::Slider smoothSlider;
    juce::Slider bandLowSlider;
    juce::Slider bandHighSlider;
    juce::Slider mixSlider;
    juce::Slider pitchSlider;
    juce::Slider grainSlider;
    juce::ToggleButton bypassButton;
    juce::ComboBox reverseModeBox;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> loopLengthAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> smoothAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> bandLowAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> bandHighAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> pitchAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> grainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> reverseModeAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ZyrinEditor)
};