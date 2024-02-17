#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_data_structures/juce_data_structures.h>

namespace juce
{
    // Based on this:
    // https://github.com/TheAudioProgrammer/jucePresetManagement
    class PresetManager : ValueTree::Listener
    {
    public:
        static const File defaultDirectory;
        static const String extension;
        static const String presetNameProperty;

        PresetManager (AudioProcessorValueTreeState&);

        void savePreset (const String& presetName);
        void deletePreset (const String& presetName);
        void loadPreset (const String& presetName);
        int loadNextPreset();
        int loadPreviousPreset();
        StringArray getAllPresets() const;
        String getCurrentPreset() const;

    private:
        void valueTreeRedirected (ValueTree& treeWhichHasBeenChanged) override;

        AudioProcessorValueTreeState& valueTreeState;
        Value currentPreset;
    };
}
