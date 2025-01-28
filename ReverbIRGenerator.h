#pragma once

#include <JuceHeader.h>

class ReverbIRGenerator
{
public:
    ReverbIRGenerator();
    ~ReverbIRGenerator() = default;

    // generates the impulse response based on the provided parameters.
    juce::AudioBuffer<float> createReverbImpulseResponse(float duration, double sampleRate, bool reverseReverb, float proximity);

private:
  // add any additional helpers here!
};
