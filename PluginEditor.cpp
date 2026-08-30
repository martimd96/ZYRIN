#include "PluginProcessor.h"
#include "PluginEditor.h"

ZyrinEditor::ZyrinEditor (ZyrinProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p) {
    // define o tamanho base da interface grafica
    setSize (400, 300);
}

ZyrinEditor::~ZyrinEditor() {}

void ZyrinEditor::paint (juce::Graphics& g) {
    // pinta o fundo de preto
    g.fillAll (juce::Colours::black);
    
    // escreve zyrin a branco bem no centro do plugin
    g.setColour (juce::Colours::white);
    g.setFont (40.0f);
    g.drawFittedText ("ZYRIN", getLocalBounds(), juce::Justification::centred, 1);
}

void ZyrinEditor::resized() {
    // aqui vao ser posicionados os knobs e botoes
}