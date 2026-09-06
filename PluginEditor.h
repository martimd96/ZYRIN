#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

// Custom LookAndFeel for futuristic dark UI
class ZyrinLookAndFeel : public juce::LookAndFeel_V4 {
public:
    ZyrinLookAndFeel() {}

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                           const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider) override;

    void drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                           float minSliderPos, float maxSliderPos, const juce::Slider::SliderStyle style, juce::Slider& slider) override;

    void drawToggleButton (juce::Graphics& g, juce::ToggleButton& button, 
                           bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
                           
    void drawComboBox (juce::Graphics& g, int width, int height, bool isButtonDown,
                       int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox& box) override;
};

// Custom attachment to link a TwoValueHorizontal slider to two APVTS parameters
class BandAttachment : private juce::Slider::Listener, private juce::AudioProcessorValueTreeState::Listener {
public:
    BandAttachment(juce::AudioProcessorValueTreeState& stateToUse, const juce::String& lowParamID, const juce::String& highParamID, juce::Slider& sliderToUse);
    ~BandAttachment() override;

private:
    void sliderValueChanged (juce::Slider* slider) override;
    void sliderDragStarted (juce::Slider* slider) override;
    void sliderDragEnded (juce::Slider* slider) override;
    void parameterChanged (const juce::String& parameterID, float newValue) override;

    juce::AudioProcessorValueTreeState& apvts;
    juce::String lowID, highID;
    juce::Slider& slider;
};

class ZyrinEditor : public juce::AudioProcessorEditor {
public:
    ZyrinEditor (ZyrinProcessor&);
    ~ZyrinEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDoubleClick (const juce::MouseEvent& event) override;

private:
    ZyrinProcessor& audioProcessor;
    ZyrinLookAndFeel customLookAndFeel;
    
    juce::TextEditor valueEditor;
    juce::Slider* currentlyEditedSlider = nullptr;
    bool editingLowBand = false;
    
    juce::ComboBox loopLengthBox;
    juce::ComboBox modeBox;
    juce::Slider smoothSlider;
    juce::Slider bandSlider;
    juce::Slider mixSlider;
    juce::Slider pitchSlider;
    juce::Slider grainSlider;
    juce::ToggleButton bypassButton;
    juce::ComboBox reverseModeBox;
    juce::Slider driveSlider;
    juce::Slider bypassFadeInSlider;
    juce::Slider bypassFadeOutSlider;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> loopLengthAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> smoothAttachment;
    
    std::unique_ptr<BandAttachment> bandAttachment;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> pitchAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> grainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> reverseModeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> driveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> bypassFadeInAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> bypassFadeOutAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ZyrinEditor)
};