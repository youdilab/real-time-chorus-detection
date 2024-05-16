/*
  ==============================================================================

    State.h
    Created: 16 May 2024 3:23:07pm
    Author:  Uditha

  ==============================================================================
*/

#pragma once

enum PluginState {
    READY,
    LISTENING,
    CALCULATING,
    BACKGROUND
};

class State
{
private:
    PluginState currState;
public:
    State()
    {
        currState = PluginState::READY;
    }

    PluginState getPluginState()
    {
        return currState;
    }

    void goToNextState()
    {
        switch (currState)
        {
        case READY:
            currState = LISTENING;
            break;
        case LISTENING:
            currState = CALCULATING;
            break;
        case CALCULATING:
            currState = BACKGROUND;
            break;
        case BACKGROUND:
            currState = READY;
            break;
        default:
            break;
        }
    }
};
