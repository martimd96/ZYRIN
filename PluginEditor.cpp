#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ZyrinAssets.h"

// --- Helper for Stacked Path Glow ---
static void drawStackedGlow(juce::Graphics& g, const juce::Path& path, juce::Colour baseColor, float maxThickness = 12.0f, int steps = 4) {
    for (int i = steps; i >= 1; --i) {
        float thickness = maxThickness * ((float)i / (float)steps);
        float alpha = (i == 1) ? 1.0f : (0.2f * (1.0f - ((float)i / (float)steps)));
        
        g.setColour(baseColor.withAlpha(alpha));
        g.strokePath(path, juce::PathStrokeType(thickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
}

// --- ZyrinLookAndFeel Implementation ---

ZyrinLookAndFeel::ZyrinLookAndFeel() {
    orbitronTypeface = juce::Typeface::createSystemTypefaceFor(ZyrinAssets::Orbitron_ttf, ZyrinAssets::Orbitron_ttfSize);
}

juce::Typeface::Ptr ZyrinLookAndFeel::getTypefaceForFont(const juce::Font& font) {
    return orbitronTypeface;
}

void ZyrinLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                                        const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider) {
    auto radius = (float) juce::jmin(width, height) / 2.0f - 8.0f;
    auto centreX = (float) x + (float) width  * 0.5f;
    auto centreY = (float) y + (float) height * 0.5f;
    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    // Background track (dark grey circle)
    g.setColour(juce::Colour(0xff1e1e26));
    g.fillEllipse(centreX - radius, centreY - radius, radius * 2.0f, radius * 2.0f);

    // Active arc with neon purple glow
    juce::Path activeArc;
    activeArc.addCentredArc(centreX, centreY, radius, radius, 0.0f, rotaryStartAngle, angle, true);
    drawStackedGlow(g, activeArc, juce::Colour(0xff9d4edd), 8.0f, 3);

    // Thumb (Clean Angled Pointer)
    juce::Path thumb;
    float thumbW = 6.0f;
    float thumbH = 10.0f;
    thumb.addTriangle(centreX, centreY - radius, 
                      centreX - thumbW*0.5f, centreY - radius + thumbH, 
                      centreX + thumbW*0.5f, centreY - radius + thumbH);
    thumb.applyTransform(juce::AffineTransform::rotation(angle, centreX, centreY));
    
    g.setColour(juce::Colours::white);
    g.fillPath(thumb);

    // Readout Text
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(12.0f).withStyle(juce::Font::bold));
    juce::String text = juce::String(slider.getValue(), 2);
    g.drawText(text, x, y + height / 2 - 10, width, 20, juce::Justification::centred, false);
}

void ZyrinLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                                        float minSliderPos, float maxSliderPos, const juce::Slider::SliderStyle style, juce::Slider& slider) {
    if (style == juce::Slider::TwoValueHorizontal) {
        auto trackHeight = 6.0f;
        auto trackY = (float) y + (height - trackHeight) * 0.5f; 

        // Background Track chamfered
        juce::Path bgTrack;
        bgTrack.addRoundedRectangle((float)x, trackY, (float)width, trackHeight, 2.0f);
        g.setColour(juce::Colour(0xff1e1e26));
        g.fillPath(bgTrack);

        // Active Band Fill with glow
        juce::Path activeBand;
        activeBand.addRoundedRectangle(minSliderPos, trackY, maxSliderPos - minSliderPos, trackHeight, 2.0f);
        
        g.setColour(juce::Colour(0xff9d4edd).withAlpha(0.4f));
        g.fillPath(activeBand);
        drawStackedGlow(g, activeBand, juce::Colour(0xff9d4edd), 6.0f, 3);

        // Thumbs (Thinner chamfered sci-fi look)
        auto drawThumb = [&](float tx) {
            juce::Path tPath;
            tPath.startNewSubPath(tx - 3.0f, trackY - 4.0f);
            tPath.lineTo(tx + 3.0f, trackY - 4.0f);
            tPath.lineTo(tx + 3.0f, trackY + trackHeight + 4.0f);
            tPath.lineTo(tx - 3.0f, trackY + trackHeight + 4.0f);
            tPath.closeSubPath();
            g.setColour(juce::Colours::white);
            g.fillPath(tPath);
        };
        
        drawThumb(minSliderPos);
        drawThumb(maxSliderPos);

        // Dynamic Frequency Labels
        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(11.0f));
        
        juce::String lowText = juce::String(slider.getMinValue(), 0) + " Hz";
        juce::String highText = juce::String(slider.getMaxValue(), 0) + " Hz";
        
        g.drawText(lowText, (int)minSliderPos - 30, (int)(trackY + trackHeight + 5.0f), 60, 15, juce::Justification::centred, false);
        g.drawText(highText, (int)maxSliderPos - 30, (int)(trackY + trackHeight + 5.0f), 60, 15, juce::Justification::centred, false);
    } else {
        auto trackY = (float) y + (float) height * 0.5f - 2.0f;
        auto trackHeight = 4.0f;

        g.setColour(juce::Colour(0xff1e1e26));
        g.fillRect(juce::Rectangle<float>((float)x, trackY, (float)width, trackHeight));

        juce::Path activeLine;
        activeLine.addLineSegment(juce::Line<float>((float)x, trackY + 2.0f, sliderPos, trackY + 2.0f), 4.0f);
        drawStackedGlow(g, activeLine, juce::Colour(0xff9d4edd), 6.0f, 3);

        g.setColour(juce::Colours::white);
        g.fillRect(juce::Rectangle<float>(sliderPos - 2.0f, trackY - 4.0f, 4.0f, 12.0f));
    }
}

void ZyrinLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button, 
                                        bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) {
    auto bounds = button.getLocalBounds().toFloat();
    auto centreX = bounds.getCentreX();
    auto centreY = bounds.getCentreY();
    auto baseRadius = 45.0f;

    bool isActive = !button.getToggleState() && isDawPlaying; // Bypass true = Plugin OFF. Also show standby if DAW stopped.

    if (isActive) {
        // Draw the BPM synced pulsing background rings (Tighter/Smaller)
        for (int i = 0; i < 3; ++i) {
            float scale = 1.0f + (currentPulsePhase * 0.2f) + (i * 0.15f);
            float r = baseRadius * scale;
            juce::Path bgHex;
            bgHex.addPolygon(juce::Point<float>((float)centreX, (float)centreY), 6, r, juce::MathConstants<float>::pi / 2.0f);
            
            float alpha = 0.3f * (1.0f - (r - baseRadius) / 30.0f);
            if (alpha < 0.0f) alpha = 0.0f;
            
            drawStackedGlow(g, bgHex, juce::Colour(0xff9d4edd).withAlpha(alpha), 3.0f, 2);
        }

        // Main Core
        juce::Path hex;
        hex.addPolygon(juce::Point<float>((float)centreX, (float)centreY), 6, baseRadius, juce::MathConstants<float>::pi / 2.0f);
        g.setColour(juce::Colour(0xff9d4edd).withAlpha(0.3f));
        g.fillPath(hex);
        drawStackedGlow(g, hex, juce::Colour(0xff9d4edd), 10.0f, 4);

        g.setColour(juce::Colours::white);
        g.setFont(juce::Font(14.0f).withStyle(juce::Font::bold));
        g.drawText("ACTIVE", bounds.toNearestInt(), juce::Justification::centred, false);
    } else {
        // Flat standby look
        juce::Path hex;
        hex.addPolygon(juce::Point<float>((float)centreX, (float)centreY), 6, baseRadius, juce::MathConstants<float>::pi / 2.0f);
        
        g.setColour(juce::Colour(0xff1e1e26)); // flat dark gray
        g.fillPath(hex);
        
        g.setColour(juce::Colour(0xff9d4edd).withAlpha(0.2f)); // very dim purple border
        g.strokePath(hex, juce::PathStrokeType(2.0f));

        g.setColour(juce::Colour(0xff555566)); // slightly dimmed text
        g.setFont(juce::Font(14.0f).withStyle(juce::Font::bold));
        g.drawText("BYPASS", bounds.toNearestInt(), juce::Justification::centred, false);
    }
}

void ZyrinLookAndFeel::drawComboBox (juce::Graphics& g, int width, int height, bool isButtonDown,
                                     int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox& box) {
    juce::Path panel;
    float chamfer = 4.0f;
    panel.startNewSubPath(chamfer, 0.0f);
    panel.lineTo((float)width - chamfer, 0.0f);
    panel.lineTo((float)width, chamfer);
    panel.lineTo((float)width, (float)height - chamfer);
    panel.lineTo((float)width - chamfer, (float)height);
    panel.lineTo(chamfer, (float)height);
    panel.lineTo(0.0f, (float)height - chamfer);
    panel.lineTo(0.0f, chamfer);
    panel.closeSubPath();

    g.setColour(juce::Colour(0xff1e1e26));
    g.fillPath(panel);
    
    g.setColour(juce::Colour(0xff9d4edd).withAlpha(0.6f));
    g.strokePath(panel, juce::PathStrokeType(1.0f));

    // Dropdown arrow
    juce::Path arrow;
    auto arrowX = (float)buttonX + (float)buttonW * 0.5f;
    auto arrowY = (float)buttonY + (float)buttonH * 0.45f;
    auto arrowW = 5.0f;
    
    arrow.addTriangle(arrowX - arrowW, arrowY, arrowX + arrowW, arrowY, arrowX, arrowY + arrowW);
    g.setColour(juce::Colour(0xff9d4edd));
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

    // Setup TextEditor for double-click input
    valueEditor.setMultiLine(false);
    valueEditor.setReturnKeyStartsNewLine(false);
    valueEditor.setReadOnly(false);
    valueEditor.setScrollbarsShown(false);
    valueEditor.setCaretVisible(true);
    valueEditor.setPopupMenuEnabled(false);
    valueEditor.setJustification(juce::Justification::centred);
    valueEditor.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff0a0a0e));
    valueEditor.setColour(juce::TextEditor::outlineColourId, juce::Colour(0xff9d4edd));
    valueEditor.setColour(juce::TextEditor::textColourId, juce::Colours::white);
    
    valueEditor.onReturnKey = [this]() {
        if (currentlyEditedSlider != nullptr) {
            float newValue = valueEditor.getText().getFloatValue();
            if (currentlyEditedSlider == &bandSlider) {
                if (editingLowBand) currentlyEditedSlider->setMinValue(newValue, juce::sendNotificationSync);
                else                currentlyEditedSlider->setMaxValue(newValue, juce::sendNotificationSync);
            } else {
                currentlyEditedSlider->setValue(newValue, juce::sendNotificationSync);
            }
        }
        valueEditor.setVisible(false);
    };
    
    valueEditor.onFocusLost = [this]() { valueEditor.setVisible(false); };
    addChildComponent(valueEditor);

    mixSlider.addMouseListener(this, false);
    pitchSlider.addMouseListener(this, false);
    grainSlider.addMouseListener(this, false);
    driveSlider.addMouseListener(this, false);
    smoothSlider.addMouseListener(this, false);
    bypassFadeInSlider.addMouseListener(this, false);
    bypassFadeOutSlider.addMouseListener(this, false);
    bandSlider.addMouseListener(this, false);

    setSize (700, 500); 
    
    startTimerHz(30);
}

ZyrinEditor::~ZyrinEditor() {
    stopTimer();
    setLookAndFeel(nullptr);
}

void ZyrinEditor::timerCallback() {
    // Inject BPM-synced phase from processor into LookAndFeel
    customLookAndFeel.currentPulsePhase = audioProcessor.currentPulsePhase.load(std::memory_order_relaxed);
    customLookAndFeel.isDawPlaying = audioProcessor.isDawPlaying.load(std::memory_order_relaxed);
    
    float currentPhase = customLookAndFeel.currentPulsePhase;
    
    // Construct dynamic oscilloscope path ONLY on beat restart to save CPU
    // and act as a static "print" that gets revealed.
    if (currentPhase < lastPulsePhase || scopePath.isEmpty()) {
        scopePath.clear();
        int pos = audioProcessor.scopePos.load(std::memory_order_relaxed);
        float midY = smoothSlider.getBottom() + 46.0f; // Mathematically centered in the gap
        float width = getWidth();
        
        for (int i = 0; i < audioProcessor.scopeSize; ++i) {
            int readPos = (pos + i) % audioProcessor.scopeSize;
            float val = audioProcessor.scopeData[readPos].load(std::memory_order_relaxed);
            float x = (float)i / (audioProcessor.scopeSize - 1) * width;
            // The signal is typically between -1 and 1. Scale it for the GUI.
            float y = midY - val * 45.0f; 
            
            if (i == 0) scopePath.startNewSubPath(x, y);
            else scopePath.lineTo(x, y);
        }
    }
    
    lastPulsePhase = currentPhase;
    
    // Repaint everything to show dynamic background and pulsing button
    repaint();
}

void ZyrinEditor::paint (juce::Graphics& g) {
    g.fillAll(juce::Colour(0xff0a0a0e));
    
    // Draw dynamic real-time oscilloscope with beat-synced wipe reveal
    g.saveState();
    float phase = customLookAndFeel.currentPulsePhase;
    int reverseMode = (int)audioProcessor.apvts.getRawParameterValue("reverseMode")->load();
    bool isReverse = reverseMode > 0;
    
    if (isReverse) {
        // Reveal from right to left
        g.reduceClipRegion((int)(getWidth() * (1.0f - phase)), 0, (int)(getWidth() * phase), getHeight());
    } else {
        // Reveal from left to right
        g.reduceClipRegion(0, 0, (int)(getWidth() * phase), getHeight());
    }
    
    g.setColour(juce::Colour(0xff9d4edd).withAlpha(0.15f));
    g.strokePath(scopePath, juce::PathStrokeType(2.0f));
    g.restoreState();
    
    // Subtle auto-bypass text when DAW is stopped
    if (!audioProcessor.isDawPlaying.load(std::memory_order_relaxed)) {
        g.setColour(juce::Colour(0xff1e1e26).withAlpha(0.8f)); // Dark gray / dim
        g.setFont(juce::Font(12.0f).withStyle(juce::Font::bold));
        
        // Exact same center point as the oscilloscope (+46 offset)
        int textCenterY = smoothSlider.getBottom() + 46; 
        g.drawText("AUTO-BYPASSED: DAW STOPPED", 0, textCenterY - 10, getWidth(), 20, juce::Justification::centred, false);
    }

    g.setColour (juce::Colours::white);
    g.setFont(juce::Font(28.0f).withStyle(juce::Font::bold));
    g.drawText ("ZYRIN", 0, 20, getWidth(), 35, juce::Justification::centred, true);

    g.setFont(12.0f);
    g.setColour(juce::Colour(0xff8A8A9A));

    auto drawLabel = [&](const juce::String& text, juce::Component& comp, int yOffset = 20) {
        g.drawText(text, comp.getX(), comp.getY() - yOffset, comp.getWidth(), 20, juce::Justification::centred, false);
    };
    
    drawLabel("LOOP", loopLengthBox, 22);
    drawLabel("MODE", modeBox, 22);
    drawLabel("REVERSE", reverseModeBox, 22);
    
    drawLabel("MIX", mixSlider);
    drawLabel("DRIVE", driveSlider);
    drawLabel("PITCH", pitchSlider);
    drawLabel("GRAIN", grainSlider);
    
    drawLabel("FADE IN", bypassFadeInSlider, 22);
    drawLabel("FADE OUT", bypassFadeOutSlider, 22);
    drawLabel("SMOOTH", smoothSlider, 22);
    
    drawLabel("BAND SPLIT", bandSlider, 25);
}

void ZyrinEditor::resized() {
    int topY = 80;
    int comboW = 120;
    int comboH = 26;
    int comboSpacing = (getWidth() - (3 * comboW)) / 4;
    
    loopLengthBox.setBounds(comboSpacing, topY, comboW, comboH);
    modeBox.setBounds(comboSpacing * 2 + comboW, topY, comboW, comboH);
    reverseModeBox.setBounds(comboSpacing * 3 + comboW * 2, topY, comboW, comboH);

    // Expand bypass button area to allow drawing the glowing aura within LookAndFeel boundaries
    int bypassSize = 200; 
    int bypassX = (getWidth() - bypassSize) / 2;
    int bypassY = (getHeight() - bypassSize) / 2 - 45; // Shifted UP by 25 pixels
    bypassButton.setBounds(bypassX, bypassY, bypassSize, bypassSize);
    
    bypassBounds = bypassButton.getBounds();

    int rotarySize = 90;
    int leftX = 50;
    int rightX = getWidth() - leftX - rotarySize;
    int row1Y = 180;

    mixSlider.setBounds(rightX, row1Y, rotarySize, rotarySize);
    pitchSlider.setBounds(leftX, row1Y, rotarySize, rotarySize);

    // Smaller inner knobs: Grain (left) and Drive (right)
    int smallRotarySize = 65;
    int innerRowY = row1Y - 15; // a little bit up
    
    // Center between pitch/mix knobs and the central bypass button
    int bypassLeft = bypassX; // 250
    int pitchRight = leftX + rotarySize; // 140
    int innerLeftX = pitchRight + (bypassLeft - pitchRight - smallRotarySize) / 2;

    int bypassRight = bypassX + bypassSize; // 450
    int mixLeft = rightX; // 560
    int innerRightX = bypassRight + (mixLeft - bypassRight - smallRotarySize) / 2;

    grainSlider.setBounds(innerLeftX, innerRowY, smallRotarySize, smallRotarySize);
    driveSlider.setBounds(innerRightX, innerRowY, smallRotarySize, smallRotarySize);

    int fadeW = 90;
    int fadeH = 18;
    int fadeY = bypassY + bypassSize - 35; // adjusted because button bounds are larger
    
    bypassFadeInSlider.setBounds(bypassButton.getBounds().getCentreX() - fadeW*1.5f - 20, fadeY, fadeW, fadeH);
    bypassFadeOutSlider.setBounds(bypassButton.getBounds().getCentreX() + fadeW*0.5f + 20, fadeY, fadeW, fadeH);
    smoothSlider.setBounds(bypassButton.getBounds().getCentreX() - fadeW*0.5f, fadeY + 30, fadeW, fadeH);

    int bandW = getWidth() - 140;
    int bandH = 45;
    int bandY = getHeight() - 65;
    bandSlider.setBounds(70, bandY, bandW, bandH);
}

void ZyrinEditor::mouseDoubleClick (const juce::MouseEvent& event) {
    auto* slider = dynamic_cast<juce::Slider*>(event.originalComponent);
    if (slider == nullptr) return;

    currentlyEditedSlider = slider;
    
    int editorW = 60;
    int editorH = 24;
    juce::Rectangle<int> editorBounds;

    if (slider == &bandSlider) {
        editingLowBand = event.x < bandSlider.getWidth() / 2;
        float currentVal = editingLowBand ? (float)slider->getMinValue() : (float)slider->getMaxValue();
        valueEditor.setText(juce::String(currentVal, 2));
        
        int cx = event.x + bandSlider.getX();
        int cy = bandSlider.getY() + bandSlider.getHeight() / 2;
        editorBounds = juce::Rectangle<int>(cx - editorW/2, cy - editorH/2, editorW, editorH);
    } else {
        valueEditor.setText(juce::String(slider->getValue(), 2));
        
        int cx = slider->getX() + slider->getWidth() / 2;
        int cy = slider->getY() + slider->getHeight() / 2;
        editorBounds = juce::Rectangle<int>(cx - editorW/2, cy - editorH/2, editorW, editorH);
    }

    valueEditor.setBounds(editorBounds);
    valueEditor.setVisible(true);
    valueEditor.grabKeyboardFocus();
}