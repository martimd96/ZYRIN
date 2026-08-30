#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

juce::AudioProcessorValueTreeState::ParameterLayout ZyrinProcessor::createParameterLayout() {
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("loopLength", 1), "Loop Length",
        juce::StringArray{"1/16", "1/8", "1/4", "1/2", "1 Bar", "2 Bars", "4 Bars", "8 Bars"}, 4));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("mode", 1), "Mode",
        juce::StringArray{"1.5x", "2x", "4x"}, 1));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("reverseMode", 1), "Reverse Mode",
        juce::StringArray{"Off", "Synced", "Instant"}, 0));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("smooth", 1), "Smooth",
        juce::NormalisableRange<float>(5.0f, 50.0f, 0.1f), 20.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("bandLow", 1), "Band Low",
        juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.3f), 20.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("bandHigh", 1), "Band High",
        juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.3f), 20000.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("mix", 1), "Mix",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 1.0f));

    params.push_back(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID("pitchShift", 1), "Pitch Shift",
        -12, 12, 0));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("grainSize", 1), "Grain Size",
        juce::NormalisableRange<float>(10.0f, 120.0f, 1.0f), 60.0f));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("bypass", 1), "Bypass", false));

    return { params.begin(), params.end() };
}

ZyrinProcessor::ZyrinProcessor()
     : AudioProcessor (BusesProperties().withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
       apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    loopLengthParam = apvts.getRawParameterValue("loopLength");
    modeParam = apvts.getRawParameterValue("mode");
    smoothParam = apvts.getRawParameterValue("smooth");
    bandLowParam = apvts.getRawParameterValue("bandLow");
    bandHighParam = apvts.getRawParameterValue("bandHigh");
    mixParam = apvts.getRawParameterValue("mix");
    pitchParam = apvts.getRawParameterValue("pitchShift");
    grainParam = apvts.getRawParameterValue("grainSize");
    bypassParam = apvts.getRawParameterValue("bypass");
    reverseModeParam = apvts.getRawParameterValue("reverseMode");

    for (int i = 0; i < 2; ++i) {
        lp1[i].setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
        hp1[i].setType(juce::dsp::LinkwitzRileyFilterType::highpass);
        lp2[i].setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
        hp2[i].setType(juce::dsp::LinkwitzRileyFilterType::highpass);
        ap1[i].setType(juce::dsp::LinkwitzRileyFilterType::allpass);
    }
}

ZyrinProcessor::~ZyrinProcessor() {}

void ZyrinProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    // 10 seconds of buffer is enough to store 1 bar at 24 BPM
    int bufferLength = static_cast<int>(sampleRate * 10.0); 
    circularBuffer.setSize(getTotalNumInputChannels(), bufferLength);
    circularBuffer.clear();
    writePosition = 0;

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 1;

    for (int i = 0; i < 2; ++i) {
        lp1[i].prepare(spec);
        hp1[i].prepare(spec);
        lp2[i].prepare(spec);
        hp2[i].prepare(spec);
        ap1[i].prepare(spec);
        
        lp1[i].reset();
        hp1[i].reset();
        lp2[i].reset();
        hp2[i].reset();
        ap1[i].reset();
    }
    
    // pitch shifter buffer for maximum 120ms grain window
    double maxWindowSamples = 0.12 * sampleRate;
    pitchBuffer.setSize(getTotalNumInputChannels(), static_cast<int>(maxWindowSamples) + 100);
    pitchBuffer.clear();
    pitchWritePos = 0;
    pitchDelayAccum = 0.0;
    
    smoothedGrainSize.reset(sampleRate, 0.05);
    smoothedGrainSize.setCurrentAndTargetValue(grainParam->load());
    
    wasInstant = false;
    instantDelaySamples = 0.0;
}

void ZyrinProcessor::releaseResources() {
    // limpa a memoria quando fechas o projeto na daw
}

void ZyrinProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    double sampleRate = getSampleRate();
    if (sampleRate <= 0.0) return;

    // Load Parameters
    int loopLengthChoice = static_cast<int>(loopLengthParam->load());
    int modeChoice = static_cast<int>(modeParam->load());
    float smoothMs = smoothParam->load();
    float lowCutoff = bandLowParam->load();
    float highCutoff = bandHighParam->load();
    float mix = mixParam->load();
    float pitchShiftSemitones = pitchParam->load();
    float grainSizeMs = grainParam->load();
    bool bypass = bypassParam->load() > 0.5f;
    int reverseModeChoice = static_cast<int>(reverseModeParam->load());

    if (lowCutoff > highCutoff) std::swap(lowCutoff, highCutoff);
    
    float fadeDelta = 1.0f / (0.02f * sampleRate); // 20ms fade

    for (int i = 0; i < 2; ++i) {
        lp1[i].setCutoffFrequency(lowCutoff);
        hp1[i].setCutoffFrequency(lowCutoff);
        lp2[i].setCutoffFrequency(highCutoff);
        hp2[i].setCutoffFrequency(highCutoff);
        ap1[i].setCutoffFrequency(highCutoff);
    }

    // Map loopLengthChoice (0-7) to beats (0.25 to 32.0)
    loopLengthBeats = 0.25 * std::pow(2.0, loopLengthChoice);

    // map modeChoice to speed where 1.5x uses 0.75 for musical jumps and perfect fifth harmony
    double speed = (modeChoice == 0) ? 0.75 : (modeChoice == 1 ? 0.5 : 0.25);
    double delayRatio = 1.0 - speed;

    double fadeSamples = smoothMs * 0.001 * sampleRate;

    // Get Transport info
    auto playHead = getPlayHead();
    double currentBpm = 120.0;
    double ppqPosition = 0.0;
    bool isPlaying = false;

    if (playHead != nullptr) {
        if (auto positionInfo = playHead->getPosition()) {
            currentBpm = positionInfo->getBpm().orFallback(120.0);
            ppqPosition = positionInfo->getPpqPosition().orFallback(0.0);
            isPlaying = positionInfo->getIsPlaying();
        }
    }
    
    int numSamples = buffer.getNumSamples();
    int bufferLength = circularBuffer.getNumSamples();
    double beatsPerSample = currentBpm / (60.0 * sampleRate);
    double fadeBeats = fadeSamples * beatsPerSample;

    if (bufferLength == 0) return;
    
    float R = std::pow(2.0f, pitchShiftSemitones / 12.0f);
    double delayDelta = 1.0 - R;
    smoothedGrainSize.setTargetValue(grainSizeMs);

    for (int sample = 0; sample < numSamples; ++sample) {
        
        // update pitch shift phase dynamically handles grain size shrinking
        double currentGrainSizeMs = smoothedGrainSize.getNextValue();
        double pitchWindowSamples = currentGrainSizeMs * 0.001 * sampleRate;
        
        pitchDelayAccum += delayDelta;
        while (pitchDelayAccum >= pitchWindowSamples) pitchDelayAccum -= pitchWindowSamples;
        while (pitchDelayAccum < 0.0) pitchDelayAccum += pitchWindowSamples;

        double delay1 = pitchDelayAccum;
        double delay2 = pitchDelayAccum + pitchWindowSamples * 0.5;
        if (delay2 >= pitchWindowSamples) delay2 -= pitchWindowSamples;

        auto getHannWindow = [](double p) {
            return 0.5 * (1.0 - std::cos(juce::MathConstants<double>::twoPi * p));
        };
        
        double p1 = delay1 / pitchWindowSamples;
        double p2 = delay2 / pitchWindowSamples;
        double w1 = getHannWindow(p1);
        double w2 = getHannWindow(p2);

        if (isPlaying && !bypass) {
            transportFade += fadeDelta;
        } else {
            transportFade -= fadeDelta;
        }
        transportFade = juce::jlimit(0.0f, 1.0f, transportFade);

        double samplePpq = ppqPosition + (isPlaying ? (sample * beatsPerSample) : 0.0);
        
        double loopPhase = std::fmod(samplePpq, loopLengthBeats);
        if (loopPhase < 0.0) loopPhase += loopLengthBeats;
        
        double delaySamplesNew = 0.0;
        double delaySamplesOld = 0.0;
        
        if (reverseModeChoice == 2) { 
            // INSTANT MODE
            if (!wasInstant) {
                instantDelaySamples = (loopPhase * delayRatio) / beatsPerSample;
                wasInstant = true;
            }
            instantDelaySamples += (1.0 + speed);
            while (instantDelaySamples >= bufferLength) instantDelaySamples -= bufferLength;
            
            delaySamplesNew = instantDelaySamples;
            delaySamplesOld = instantDelaySamples; 
        } else {
            wasInstant = false;
            if (reverseModeChoice == 1) { 
                // SYNCED MODE
                double syncRatio = 1.0 + speed;
                double delayBeatsNew = loopPhase * syncRatio;
                double delayBeatsOld = (loopPhase + loopLengthBeats) * syncRatio;
                
                delaySamplesNew = delayBeatsNew / beatsPerSample;
                delaySamplesOld = delayBeatsOld / beatsPerSample;
            } else { 
                // OFF MODE
                double delayBeatsNew = loopPhase * delayRatio;
                double delayBeatsOld = (loopPhase + loopLengthBeats) * delayRatio;
                
                delaySamplesNew = delayBeatsNew / beatsPerSample;
                delaySamplesOld = delayBeatsOld / beatsPerSample;
            }
        }

        float fade = 1.0f;
        if (loopPhase < fadeBeats && fadeBeats > 0.0) {
            fade = static_cast<float>(loopPhase / fadeBeats);
        }

        for (int channel = 0; channel < totalNumInputChannels; ++channel) {
            auto* inputData = buffer.getReadPointer(channel);
            auto* outputData = buffer.getWritePointer(channel);
            auto* circularData = circularBuffer.getWritePointer(channel);
            
            float inputSample = inputData[sample];

            // Band Split
            float low = lp1[channel].processSample(0, inputSample);
            float midHigh = hp1[channel].processSample(0, inputSample);
            float mid = lp2[channel].processSample(0, midHigh);
            float high = hp2[channel].processSample(0, midHigh);
            
            float lowAligned = ap1[channel].processSample(0, low);
            
            // Write 'mid' band to circular buffer
            circularData[writePosition] = mid;
            
            // Read new loop
            auto getInterpolated = [&](double dSamples) {
                double readPosition = writePosition - dSamples;
                while (readPosition < 0.0) readPosition += bufferLength;
                while (readPosition >= bufferLength) readPosition -= bufferLength;
                
                int index1 = static_cast<int>(std::floor(readPosition));
                int index2 = (index1 + 1) % bufferLength;
                double fraction = readPosition - index1;
                
                return circularData[index1] * (1.0f - fraction) + circularData[index2] * fraction;
            };

            float newRead = getInterpolated(delaySamplesNew);
            float oldRead = getInterpolated(delaySamplesOld);

            float stretchedMid = fade * newRead + (1.0f - fade) * oldRead;
            
            // Apply Pitch Shift to stretchedMid
            float shiftedMid = stretchedMid;
            if (pitchShiftSemitones != 0.0f) {
                auto* pitchData = pitchBuffer.getWritePointer(channel);
                pitchData[pitchWritePos] = stretchedMid;
                
                auto getPitchInterpolated = [&](double delay) {
                    int len = pitchBuffer.getNumSamples();
                    double rPos = pitchWritePos - delay - 1.0;
                    while (rPos < 0.0) rPos += len;
                    while (rPos >= len) rPos -= len;
                    
                    int idx1 = static_cast<int>(std::floor(rPos));
                    int idx2 = (idx1 + 1) % len;
                    double frac = rPos - idx1;
                    
                    return pitchData[idx1] * (1.0f - frac) + pitchData[idx2] * frac;
                };
                
                shiftedMid = static_cast<float>(getPitchInterpolated(delay1) * w1 + getPitchInterpolated(delay2) * w2);
            }

            float wetSample = lowAligned + shiftedMid + high;

            float effectiveMix = mix * transportFade;
            outputData[sample] = inputSample * (1.0f - effectiveMix) + wetSample * effectiveMix;
        }
        writePosition = (writePosition + 1) % bufferLength;
        pitchWritePos = (pitchWritePos + 1) % pitchBuffer.getNumSamples();
    }
}

juce::AudioProcessorEditor* ZyrinProcessor::createEditor() {
    return new ZyrinEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new ZyrinProcessor();
}