#include "PluginProcessor.h"
#include "PluginEditor.h"

// --- ZyrinLookAndFeel Implementation ---

void ZyrinLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                                        const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider) {
    auto radius = (float) juce::jmin(width, height) / 2.0f - 4.0f;
    auto centreX = (float) x + (float) width  * 0.5f;
    auto centreY = (float) y + (float) height * 0.5f;
    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    // Background circle
    g.setColour(juce::Colour(0xff1e1e26));
    g.fillEllipse(centreX - radius, centreY - radius, radius * 2.0f, radius * 2.0f);

    // Active arc (stroked path)
    g.setColour(juce::Colour(0xff9d4edd));
    juce::Path arcPath;
    arcPath.addCentredArc(centreX, centreY, radius, radius, 0.0f, rotaryStartAngle, angle, true);
    g.strokePath(arcPath, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Pointer line
    juce::Path pointer;
    pointer.startNewSubPath(centreX, centreY);
    pointer.lineTo(centreX + radius * std::sin(angle), centreY - radius * std::cos(angle));
    g.setColour(juce::Colours::white);
    g.strokePath(pointer, juce::PathStrokeType(2.0f));
}

void ZyrinLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                                        float minSliderPos, float maxSliderPos, const juce::Slider::SliderStyle style, juce::Slider& slider) {
    auto trackY = (float) y + (float) height * 0.5f - 4.0f;
    auto trackHeight = 8.0f;

    // Background Track
    g.setColour(juce::Colour(0xff1e1e26));
    g.fillRect(juce::Rectangle<float>((float)x, trackY, (float)width, trackHeight));

    if (style == juce::Slider::TwoValueHorizontal) {
        // Active Band Fill
        g.setColour(juce::Colour(0xff9d4edd));
        g.fillRect(juce::Rectangle<float>(minSliderPos, trackY, maxSliderPos - minSliderPos, trackHeight));

        // Thumbs
        g.setColour(juce::Colours::white);
        auto thumbHeight = 16.0f;
        auto thumbY = (float) y + ((float) height - thumbHeight) * 0.5f;
        g.fillRect(juce::Rectangle<float>(minSliderPos - 2.0f, thumbY, 4.0f, thumbHeight));
        g.fillRect(juce::Rectangle<float>(maxSliderPos - 2.0f, thumbY, 4.0f, thumbHeight));
    } else {
        // Normal linear slider handling (fade in/out, smooth)
        g.setColour(juce::Colour(0xff9d4edd));
        g.fillRect(juce::Rectangle<float>((float)x, trackY, sliderPos - (float)x, trackHeight));

        g.setColour(juce::Colours::white);
        g.fillRect(juce::Rectangle<float>(sliderPos - 2.0f, trackY - 4.0f, 4.0f, trackHeight + 8.0f));
    }
}

void ZyrinLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button, 
                                        bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) {
    auto bounds = button.getLocalBounds().toFloat();
    auto centreX = bounds.getCentreX();
    auto centreY = bounds.getCentreY();
    auto radius = 35.0f;

    bool isActive = !button.getToggleState(); // Bypass true = Plugin OFF

    if (shouldDrawButtonAsDown || isActive) {
        g.setColour(juce::Colour(0xff9d4edd)); // Active/Down
    } else {
        g.setColour(juce::Colour(0xff1e1e26)); // Bypassed
    }
    g.fillEllipse(centreX - radius, centreY - radius, radius * 2.0f, radius * 2.0f);

    // White power symbol
    g.setColour(juce::Colours::white);
    auto symbolRadius = radius * 0.5f;
    juce::Path powerCircle;
    powerCircle.addCentredArc(centreX, centreY, symbolRadius, symbolRadius, 0.0f, 
                              juce::MathConstants<float>::pi * 0.25f, 
                              juce::MathConstants<float>::pi * 1.75f, true);
    g.strokePath(powerCircle, juce::PathStrokeType(3.0f, juce::PathStrokeType::mitered, juce::PathStrokeType::rounded));
    g.drawLine(centreX, centreY - symbolRadius * 1.2f, centreX, centreY, 3.0f);
}

void ZyrinLookAndFeel::drawComboBox (juce::Graphics& g, int width, int height, bool isButtonDown,
                                     int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox& box) {
    juce::Rectangle<float> bounds(0.0f, 0.0f, (float)width, (float)height);
    
    g.setColour(juce::Colour(0xff1e1e26));
    g.fillRect(bounds);
    
    g.setColour(juce::Colour(0xff9d4edd));
    g.drawRect(bounds, 1.0f);
    
    // Dropdown arrow
    juce::Path arrow;
    auto arrowX = (float)buttonX + (float)buttonW * 0.5f;
    auto arrowY = (float)buttonY + (float)buttonH * 0.4f;
    auto arrowW = 8.0f;
    
    arrow.startNewSubPath(arrowX - arrowW * 0.5f, arrowY);
    arrow.lineTo(arrowX + arrowW * 0.5f, arrowY);
    arrow.lineTo(arrowX, arrowY + arrowW * 0.6f);
    arrow.closeSubPath();
    
    g.setColour(juce::Colours::white);
    g.fillPath(arrow);
}

// --- BandAttachment Implementation ---

BandAttachment::BandAttachment(juce::AudioProcessorValueTreeState& stateToUse, const juce::String& lowParamID, const juce::String& highParamID, juce::Slider& sliderToUse)
    : apvts(stateToUse), lowID(lowParamID), highID(highParamID), slider(sliderToUse)
{
    apvts.addParameterListener(lowID, this);
    apvts.addParameterListener(highID, this);
    slider.addListener(this);
    
    auto* lowParam = apvts.getParameter(lowID);
    auto* highParam = apvts.getParameter(highID);
    
    if (lowParam != nullptr && highParam != nullptr) {
        auto lowRange = lowParam->getNormalisableRange();
        slider.setRange(lowRange.start, lowRange.end);
        
        slider.setMinAndMaxValues(*apvts.getRawParameterValue(lowID), 
                                  *apvts.getRawParameterValue(highID), 
                                  juce::dontSendNotification);
    }
}

BandAttachment::~BandAttachment() {
    slider.removeListener(this);
    apvts.removeParameterListener(lowID, this);
    apvts.removeParameterListener(highID, this);
}

void BandAttachment::sliderValueChanged(juce::Slider* s) {
    float newLow = (float)s->getMinValue();
    float newHigh = (float)s->getMaxValue();
    
    if (auto* lowParam = apvts.getParameter(lowID))
        lowParam->setValueNotifyingHost(lowParam->convertTo0to1(newLow));
    if (auto* highParam = apvts.getParameter(highID))
        highParam->setValueNotifyingHost(highParam->convertTo0to1(newHigh));
}

void BandAttachment::sliderDragStarted(juce::Slider*) {
    if (auto* lowParam = apvts.getParameter(lowID)) lowParam->beginChangeGesture();
    if (auto* highParam = apvts.getParameter(highID)) highParam->beginChangeGesture();
}

void BandAttachment::sliderDragEnded(juce::Slider*) {
    if (auto* lowParam = apvts.getParameter(lowID)) lowParam->endChangeGesture();
    if (auto* highParam = apvts.getParameter(highID)) highParam->endChangeGesture();
}

void BandAttachment::parameterChanged(const juce::String& parameterID, float newValue) {
    juce::Component::SafePointer<juce::Slider> safeSlider (&slider);
    juce::String lID = lowID;
    juce::String hID = highID;
    
    juce::MessageManager::callAsync([safeSlider, parameterID, newValue, lID, hID]() {
        if (safeSlider != nullptr) {
            if (parameterID == lID) {
                safeSlider->setMinValue(newValue, juce::dontSendNotification, false);
            } else if (parameterID == hID) {
                safeSlider->setMaxValue(newValue, juce::dontSendNotification, false);
            }
        }
    });
}

// --- ZyrinEditor Implementation ---

ZyrinEditor::ZyrinEditor (ZyrinProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p) {
    
    setLookAndFeel(&customLookAndFeel);

    // Setup ComboBoxes
    addAndMakeVisible(loopLengthBox);
    loopLengthBox.addItemList({"1/16", "1/8", "1/4", "1/2", "1 Bar", "2 Bars", "4 Bars", "8 Bars"}, 1);
    loopLengthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.apvts, "loopLength", loopLengthBox);

    addAndMakeVisible(modeBox);
    modeBox.addItemList({"1.5x", "2x", "4x"}, 1);
    modeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.apvts, "mode", modeBox);

    addAndMakeVisible(reverseModeBox);
    reverseModeBox.addItemList({"Off", "Synced", "Instant"}, 1);
    reverseModeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.apvts, "reverseMode", reverseModeBox);

    // Setup Rotary Sliders
    auto setupRotary = [this](juce::Slider& s) {
        addAndMakeVisible(s);
        s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    };

    setupRotary(mixSlider);
    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "mix", mixSlider);

    setupRotary(pitchSlider);
    pitchAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "pitchShift", pitchSlider);

    setupRotary(grainSlider);
    grainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "grainSize", grainSlider);
    
    setupRotary(driveSlider);
    driveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "drive", driveSlider);

    // Setup Linear Sliders
    auto setupLinear = [this](juce::Slider& s) {
        addAndMakeVisible(s);
        s.setSliderStyle(juce::Slider::LinearHorizontal);
        s.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    };

    setupLinear(smoothSlider);
    smoothAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "smooth", smoothSlider);
    
    setupLinear(bypassFadeInSlider);
    bypassFadeInAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "bypassFadeIn", bypassFadeInSlider);

    setupLinear(bypassFadeOutSlider);
    bypassFadeOutAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "bypassFadeOut", bypassFadeOutSlider);

    // Setup Band Slider (TwoValueHorizontal)
    addAndMakeVisible(bandSlider);
    bandSlider.setSliderStyle(juce::Slider::TwoValueHorizontal);
    bandSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    bandAttachment = std::make_unique<BandAttachment>(audioProcessor.apvts, "bandLow", "bandHigh", bandSlider);

    // Setup Bypass Button
    addAndMakeVisible(bypassButton);
    bypassButton.setButtonText("");
    bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.apvts, "bypass", bypassButton);

    setSize (600, 450);
}

ZyrinEditor::~ZyrinEditor() {
    setLookAndFeel(nullptr);
}

void ZyrinEditor::paint (juce::Graphics& g) {
    g.fillAll (juce::Colour(0xff0a0a0e));
    
    g.setColour (juce::Colours::white);
    
    // Bold, large font for title avoiding deprecated constructor
    g.setFont(juce::Font(30.0f).withStyle(juce::Font::bold));
    g.drawText ("ZYRIN", 0, 15, getWidth(), 35, juce::Justification::centred, true);

    // Reset font for labels
    g.setFont(14.0f);
    g.setColour(juce::Colour(0xffa0a0a0));

    // Draw Labels based on component bounds
    auto drawLabel = [&](const juce::String& text, juce::Component& comp, int yOffset = 20) {
        g.drawText(text, comp.getX(), comp.getY() - yOffset, comp.getWidth(), 20, juce::Justification::centred, false);
    };
    
    // Top Row Labels
    drawLabel("Loop", loopLengthBox, 22);
    drawLabel("Mode", modeBox, 22);
    drawLabel("Reverse", reverseModeBox, 22);
    
    // Rotary Labels
    drawLabel("Mix", mixSlider);
    drawLabel("Drive", driveSlider);
    drawLabel("Pitch", pitchSlider);
    drawLabel("Grain", grainSlider);
    
    // Fade/Smooth Labels
    drawLabel("Fade In", bypassFadeInSlider);
    drawLabel("Fade Out", bypassFadeOutSlider);
    drawLabel("Smooth", smoothSlider);
    
    // Band Slider Label
    drawLabel("Band Split (Low - High)", bandSlider);
}

void ZyrinEditor::resized() {
    // Top Row: 3 ComboBoxes (Loop, Mode, Reverse) horizontally across top
    int topY = 70;
    int comboW = 100;
    int comboH = 24;
    int comboSpacing = (getWidth() - (3 * comboW)) / 4;
    
    loopLengthBox.setBounds(comboSpacing, topY, comboW, comboH);
    modeBox.setBounds(comboSpacing * 2 + comboW, topY, comboW, comboH);
    reverseModeBox.setBounds(comboSpacing * 3 + comboW * 2, topY, comboW, comboH);

    // Center: Massive Bypass ToggleButton
    int bypassSize = 100;
    int bypassX = (getWidth() - bypassSize) / 2;
    int bypassY = (getHeight() - bypassSize) / 2 - 10;
    bypassButton.setBounds(bypassX, bypassY, bypassSize, bypassSize);

    // Left Side: Mix and Drive
    int rotarySize = 80;
    int leftX = 60;
    int rightX = getWidth() - leftX - rotarySize;
    int row1Y = 160;
    int row2Y = 270;

    mixSlider.setBounds(leftX, row1Y, rotarySize, rotarySize);
    driveSlider.setBounds(leftX, row2Y, rotarySize, rotarySize);

    // Right Side: Pitch and Grain
    pitchSlider.setBounds(rightX, row1Y, rotarySize, rotarySize);
    grainSlider.setBounds(rightX, row2Y, rotarySize, rotarySize);

    // Flanking the Center: bypassFadeIn (left), bypassFadeOut (right), smoothSlider (below center)
    int fadeW = 80;
    int fadeH = 16;
    int fadeY = bypassY + bypassSize + 10;
    
    bypassFadeInSlider.setBounds(bypassX - fadeW - 30, fadeY, fadeW, fadeH);
    bypassFadeOutSlider.setBounds(bypassX + bypassSize + 30, fadeY, fadeW, fadeH);
    smoothSlider.setBounds((getWidth() - fadeW) / 2, fadeY + 30, fadeW, fadeH);

    // Bottom Span: Band Split slider
    int bandW = getWidth() - 100;
    int bandH = 24;
    int bandY = getHeight() - 50;
    bandSlider.setBounds(50, bandY, bandW, bandH);
}