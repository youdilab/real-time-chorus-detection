/*
  ==============================================================================

    Config.h
    Created: 16 May 2024 11:54:36am
    Author:  Uditha

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "State.h"

namespace parameters
{
    struct audioParameterFloat
    {
        const juce::String id;
        const juce::String name;
        const float minValue;
        const float maxValue;
        const float defaultValue;
    };

    struct audioParameterInt
    {
        const juce::String id;
        const juce::String name;
        const int minValue;
        const int maxValue;
        const int defaultValue;
    };

    static const audioParameterFloat volume1{"volume1", "Volume 1", 0, 100, 50};
    static const audioParameterFloat volume2{"volume2", "Volume 2", 0, 100, 50};
    static const audioParameterFloat chorusAmount{"chorusAmount", "Chorus Amount", 0, 100, 50};
    static const audioParameterInt chorusState{"state", "State", 0, 2, 0};
}

namespace text
{
    static const juce::String textEval{"Evaluate"};
    static const juce::String textStatus{"Status"};
    static const juce::String textSolo{"Solo"};
    static const juce::String textChorus{"Chorus"};
}

namespace api
{
    static const juce::String url{"http://127.0.0.1:8000/process-audio/"};
}