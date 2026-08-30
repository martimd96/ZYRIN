#include "PluginProcessor.h"
#include "PluginEditor.h"

ZyrinEditor::ZyrinEditor (ZyrinProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p) {
    
    addAndMakeVisible(loopLengthBox);
    loopLengthBox.addItemList({"1/16", "1/8", "1/4", "1/2", "1 Bar", "2 Bars", "4 Bars", "8 Bars"}, 1);
    loopLengthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.apvts, "loopLength", loopLengthBox);

    addAndMakeVisible(modeBox);
    modeBox.addItemList({"1.5x", "2x", "4x"}, 1);
    modeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.apvts, "mode", modeBox);

    addAndMakeVisible(smoothSlider);
    smoothSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    smoothAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "smooth", smoothSlider);

    addAndMakeVisible(bandLowSlider);
    bandLowSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    bandLowAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "bandLow", bandLowSlider);

    addAndMakeVisible(bandHighSlider);
    bandHighSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    bandHighAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "bandHigh", bandHighSlider);

    addAndMakeVisible(mixSlider);
    mixSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "mix", mixSlider);

    addAndMakeVisible(pitchSlider);
    pitchSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    pitchAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "pitchShift", pitchSlider);

    addAndMakeVisible(grainSlider);
    grainSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    grainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "grainSize", grainSlider);

    addAndMakeVisible(bypassButton);
    bypassButton.setButtonText("Bypass");
    bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.apvts, "bypass", bypassButton);

    addAndMakeVisible(reverseModeBox);
    reverseModeBox.addItemList({"Off", "Synced", "Instant"}, 1);
    reverseModeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.apvts, "reverseMode", reverseModeBox);

    addAndMakeVisible(driveSlider);
    driveSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    driveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "drive", driveSlider);

    setSize (400, 450);
}

ZyrinEditor::~ZyrinEditor() {}

void ZyrinEditor::paint (juce::Graphics& g) {
    g.fillAll (juce::Colours::black);
    
    g.setColour (juce::Colours::white);
    g.setFont (20.0f);
    g.drawText ("ZYRIN - TEST UI", getLocalBounds().removeFromTop(40), juce::Justification::centred, 1);
    
    g.setFont(12.0f);
    g.drawText("Loop", 10, 50, 80, 20, juce::Justification::centredLeft);
    g.drawText("Mode", 10, 80, 80, 20, juce::Justification::centredLeft);
    g.drawText("Smooth", 10, 110, 80, 20, juce::Justification::centredLeft);
    g.drawText("Low", 10, 140, 80, 20, juce::Justification::centredLeft);
    g.drawText("High", 10, 170, 80, 20, juce::Justification::centredLeft);
    g.drawText("Mix", 10, 200, 80, 20, juce::Justification::centredLeft);
    g.drawText("Pitch", 10, 230, 80, 20, juce::Justification::centredLeft);
    g.drawText("Grain", 10, 260, 80, 20, juce::Justification::centredLeft);
    g.drawText("Bypass", 10, 290, 80, 20, juce::Justification::centredLeft);
    g.drawText("Reverse", 10, 320, 80, 20, juce::Justification::centredLeft);
    g.drawText("Drive", 10, 350, 80, 20, juce::Justification::centredLeft);
}

void ZyrinEditor::resized() {
    loopLengthBox.setBounds(100, 50, 280, 20);
    modeBox.setBounds(100, 80, 280, 20);
    smoothSlider.setBounds(100, 110, 280, 20);
    bandLowSlider.setBounds(100, 140, 280, 20);
    bandHighSlider.setBounds(100, 170, 280, 20);
    mixSlider.setBounds(100, 200, 280, 20);
    pitchSlider.setBounds(100, 230, 280, 20);
    grainSlider.setBounds(100, 260, 280, 20);
    bypassButton.setBounds(100, 290, 280, 20);
    reverseModeBox.setBounds(100, 320, 280, 20);
    driveSlider.setBounds(100, 350, 280, 20);
}