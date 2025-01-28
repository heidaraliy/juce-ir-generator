#include "ReverbIRGenerator.h"

ReverbIRGenerator::ReverbIRGenerator()
{
    // init if needed!
}

juce::AudioBuffer<float> ReverbIRGenerator::createReverbImpulseResponse(float duration, double sampleRate, bool reverseReverb, float proximity)
{
    const int length = static_cast<int>(sampleRate * duration);
    juce::AudioBuffer<float> impulseResponse(2, length);
    impulseResponse.clear();

    // early reflections -- they're hardcoded, but can be dynamically generated too! a good starting point for a decent
    // early reflection implementation.
    const int numEarlyReflections = 12;
    float earlyDelaysMs[numEarlyReflections] = {7.0f, 11.0f, 13.0f, 17.0f, 23.0f, 29.0f, 31.0f, 37.0f, 41.0f, 43.0f, 47.0f, 53.0f};
    float earlyGains[numEarlyReflections];
    for (int i = 0; i < numEarlyReflections; ++i)
        earlyGains[i] = juce::Random::getSystemRandom().nextFloat() * 0.5f + 0.5f;

    for (int i = 0; i < numEarlyReflections; ++i)
    {
        int delaySamples = static_cast<int>(earlyDelaysMs[i] * sampleRate / 1000.0f);
        if (delaySamples < length)
        {
            float g = earlyGains[i];
            float signL = (juce::Random::getSystemRandom().nextBool() ? 1.0f : -1.0f);
            float signR = (juce::Random::getSystemRandom().nextBool() ? 1.0f : -1.0f);
            impulseResponse.setSample(0, delaySamples, g * signL);
            impulseResponse.setSample(1, delaySamples, g * signR);
        }
    }

    // late reverb: continuous noise with exponential decay.
    const int lateStart = static_cast<int>(0.1 * sampleRate);
    for (int i = lateStart; i < length; ++i)
    {
        float t = static_cast<float>(i) / static_cast<float>(sampleRate);
        float decayEnv = std::exp(-6.91f * t / duration);

        // generate noise: small random fluctuations for each channel.
        float nL = (juce::Random::getSystemRandom().nextFloat() * 2.0f - 1.0f) * 0.5f;
        float nR = (juce::Random::getSystemRandom().nextFloat() * 2.0f - 1.0f) * 0.5f;

        impulseResponse.setSample(0, i, nL * decayEnv);
        impulseResponse.setSample(1, i, nR * decayEnv);
    }

    // gentle amplitude modulation instead of re-indexing.
    float modDepth = 0.05f;   // 5% amplitude variation.
    float modRate = 0.1f;     // slow modulation rate, 0.1 Hz.
    for (int c = 0; c < impulseResponse.getNumChannels(); ++c)
    {
        auto* data = impulseResponse.getWritePointer(c);
        for (int i = lateStart; i < length; ++i)
        {
            float t = static_cast<float>(i) / static_cast<float>(sampleRate);
            float mod = 1.0f + modDepth * std::sin(2.0f * juce::MathConstants<float>::pi * modRate * t);
            data[i] *= mod;
        }
    }

    // adjust the proximity: early vs. late energy.
    proximity = juce::jlimit(0.0f, 1.0f, proximity / 100.0f);
    float earlyGain = juce::jmap(proximity, 1.0f, 0.0f);
    float lateGain = juce::jmap(proximity, 0.5f, 1.0f);
    for (int c = 0; c < impulseResponse.getNumChannels(); ++c)
    {
        auto* data = impulseResponse.getWritePointer(c);
        for (int i = 0; i < length; ++i)
        {
            if (i < lateStart)
                data[i] *= earlyGain;
            else
                data[i] *= lateGain;
        }
    }

    // this reverses the entire IR buffer -- pretty cool little easter egg to create swelling effects vs. decaying ones.
    if (reverseReverb)
    {
        for (int channel = 0; channel < impulseResponse.getNumChannels(); ++channel)
        {
            auto* data = impulseResponse.getWritePointer(channel);
            std::reverse(data, data + length);
        }

        // analyze the first 100ms of reversed IR and check the volume.
        int checkSamples = static_cast<int>(0.1 * sampleRate);
        checkSamples = juce::jmin(checkSamples, length);

        float startMax = 0.0f;
        for (int c = 0; c < impulseResponse.getNumChannels(); ++c)
        {
            auto* d = impulseResponse.getReadPointer(c);
            for (int i = 0; i < checkSamples; ++i)
                startMax = juce::jmax(startMax, std::abs(d[i]));
        }

        // boost the entire IR if the start is too quiet -- works especially well with longer decay times.
        if (startMax < 0.9f && startMax > 0.0f)
        {
            float boost = 0.9f / startMax;
            impulseResponse.applyGain(boost);
        }
    }

    // finally, normalize the IR if needed.
    float maxAmp = 0.0f;
    for (int c = 0; c < impulseResponse.getNumChannels(); ++c)
        maxAmp = juce::jmax(maxAmp, impulseResponse.getMagnitude(c, 0, length));

    if (maxAmp > 0.0f)
        impulseResponse.applyGain(1.0f / maxAmp);

    return impulseResponse;
}
