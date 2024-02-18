#pragma once

#include <juce_data_structures/juce_data_structures.h>

// Manages the state of the OSCBridge
// Each channel in the bridge is represented by a ValueTree
// The ValueTree is used to store the state of the channel
// The state of the channel is used to create/set the OSCBridgeChannel
// TODO: Make this use AsyncUpdater to optimize for realtime
class LambdaStateListener : public juce::ValueTree::Listener
{
public:
    // Constructor
    LambdaStateListener (juce::ValueTree allChannels)
        : mChannelState (allChannels)
    {
        allChannels.addListener (this);
    }

    // Destructor
    ~LambdaStateListener() override
    {
        mChannelState.removeListener (this);
    }

    // Called when the state of the ValueTree changes
    void valueTreePropertyChanged (juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override
    {
        juce::Logger::writeToLog ("ValueTree property changed: " + property.toString());
    }

    void setChangedCallback (std::function<void()> callback)
    {
        mChangedCallback = callback;
    }

    // Called when a child is added to the ValueTree
    void valueTreeChildAdded (juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenAdded) override
    {
        juce::Logger::writeToLog ("ValueTree child added: " + childWhichHasBeenAdded.getType().toString());
    }

    // Called when a child is removed from the ValueTree
    void valueTreeChildRemoved (juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenRemoved, int indexFromWhichChildWasRemoved) override
    {
        juce::Logger::writeToLog ("ValueTree child removed: " + childWhichHasBeenRemoved.getType().toString());
    }

    // Called when a child is moved in the ValueTree
    void valueTreeChildOrderChanged (juce::ValueTree& parentTreeWhoseChildrenHaveMoved, int oldIndex, int newIndex) override
    {
        juce::Logger::writeToLog ("ValueTree child order changed");
    }

    // Called when a ValueTree is removed
    void valueTreeParentChanged (juce::ValueTree& treeWhoseParentHasChanged) override
    {
        juce::Logger::writeToLog ("ValueTree parent changed");
    }

    // Called when a ValueTree is removed
    void valueTreeRedirected (juce::ValueTree& treeWhichHasBeenChanged) override
    {
        juce::Logger::writeToLog ("ValueTree redirected");
    }

    void setState (juce::ValueTree newState)
    {
        if (newState.isValid())
        {
            mChannelState.removeListener (this);
            mChannelState = newState;
            mChannelState.addListener (this);
        }
    }

private:
    juce::ValueTree mChannelState;

    // Callbacks
    std::function<void()> mChangedCallback;
};
