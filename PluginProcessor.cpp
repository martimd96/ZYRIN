#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

ZyrinProcessor::ZyrinProcessor()
     : AudioProcessor (BusesProperties().withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)) {}

ZyrinProcessor::~ZyrinProcessor() {}

void ZyrinProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    // 10 seconds of buffer is enough to store 1 bar at 24 BPM
    int bufferLength = static_cast<int>(sampleRate * 10.0); 
    circularBuffer.setSize(getTotalNumInputChannels(), bufferLength);
    circularBuffer.clear();
    writePosition = 0;
}

void ZyrinProcessor::releaseResources() {
    // limpa a memoria quando fechas o projeto na daw
}

void ZyrinProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // limpa canais vazios para evitar barulhos passivos
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    double sampleRate = getSampleRate();
    if (sampleRate <= 0.0) return;

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

    // If buffer is empty/invalid, skip
    if (bufferLength == 0) return;

    for (int sample = 0; sample < numSamples; ++sample) {
        // Calculate the exact PPQ position for this sample
        double samplePpq = ppqPosition + (isPlaying ? (sample * beatsPerSample) : 0.0);
        
        // Loop phase (0.0 to loopLengthBeats)
        double loopPhase = std::fmod(samplePpq, loopLengthBeats);
        if (loopPhase < 0.0) loopPhase += loopLengthBeats;
        
        // Delay distance in beats is exactly half the loop phase.
        // This ensures the read pointer moves at 0.5x speed.
        double delayBeats = loopPhase * 0.5;
        double delaySamples = delayBeats / beatsPerSample;
        
        for (int channel = 0; channel < totalNumInputChannels; ++channel) {
            auto* inputData = buffer.getReadPointer(channel);
            auto* outputData = buffer.getWritePointer(channel);
            auto* circularData = circularBuffer.getWritePointer(channel);
            
            // Write current sample to circular buffer
            circularData[writePosition] = inputData[sample];
            
            // Calculate read position
            double readPosition = writePosition - delaySamples;
            while (readPosition < 0.0) readPosition += bufferLength;
            while (readPosition >= bufferLength) readPosition -= bufferLength;
            
            // Linear Interpolation
            int index1 = static_cast<int>(std::floor(readPosition));
            int index2 = (index1 + 1) % bufferLength;
            double fraction = readPosition - index1;
            
            float interpolatedSample = circularData[index1] * (1.0f - fraction) + circularData[index2] * fraction;
            
            // Output
            outputData[sample] = interpolatedSample;
        }
        
        // Advance write pointer
        writePosition = (writePosition + 1) % bufferLength;
    }
}

juce::AudioProcessorEditor* ZyrinProcessor::createEditor() {
    return new ZyrinEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new ZyrinProcessor();
}