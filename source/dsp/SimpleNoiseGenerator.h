#pragma once

#include <juce_dsp/juce_dsp.h>

class SimpleNoiseGenerator
{
public:
    SimpleNoiseGenerator() = default;

    // Generate a single noise sample
    float generateSample()
    {
        return random.nextFloat() * 2.0f - 1.0f; // Generates a float between -1.0 and 1.0
    }

    // Fill a buffer with noise
    void fillBufferWithNoise (juce::AudioBuffer<float>& buffer, auto level = 0.5f)
    {
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        {
            float* channelData = buffer.getWritePointer (channel);
            for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            {
                channelData[sample] = generateSample() * level;
            }
        }
    }

private:
    juce::Random random; // JUCE random number generator
};
