#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

namespace birdhouse
{

    /**
   * @class TextEditorAttachment
   * @brief The TextEditorAttachment class is a helper class to attach a TextEditor to an AudioParameterFloat or AudioParameterInt.
   *
   */
    template <typename ParamType>
    class TextEditorAttachment
    {
    public:
        TextEditorAttachment (juce::RangedAudioParameter* rangedAudioParam, juce::TextEditor& editor, juce::UndoManager* undoManager = nullptr)
            : mTextEditor (editor), mUndoManager (undoManager)
        {
            if constexpr (std::is_same<ParamType, juce::AudioParameterFloat>::value)
            {
                setupForFloatParameter (rangedAudioParam);
            }
            else if constexpr (std::is_same<ParamType, juce::AudioParameterInt>::value)
            {
                setupForIntParameter (rangedAudioParam);
            }
            else
            {
                jassertfalse;
            }
        }

    private:
        juce::TextEditor& mTextEditor;
        juce::UndoManager* mUndoManager;
        std::unique_ptr<juce::ParameterAttachment> attachment;

        void setupForFloatParameter (auto parameter)
        {
            attachment = std::make_unique<juce::ParameterAttachment> (
                *parameter, [&] (float newValue) {
                    juce::ignoreUnused (parameter);
                    mTextEditor.setText (juce::String (newValue), juce::dontSendNotification);
                },
                mUndoManager);

            mTextEditor.onTextChange = [this, &parameter] {
                auto string = mTextEditor.getText();
                auto value = string.getFloatValue();
                auto normalized = parameter->convertTo0to1 (value);
                parameter->setValueNotifyingHost (normalized);
            };
        }

        void setupForIntParameter (auto parameter)
        {
            attachment = std::make_unique<juce::ParameterAttachment> (
                *parameter, [&] (float newValue) {
                    juce::ignoreUnused (parameter);
                    mTextEditor.setText (juce::String (static_cast<int> (newValue)), juce::dontSendNotification);
                },
                mUndoManager);

            mTextEditor.onTextChange = [this, &parameter] {
                auto string = mTextEditor.getText();
                auto value = string.getIntValue();
                auto normalized = parameter->convertTo0to1 (value);
                parameter->setValueNotifyingHost (normalized);
            };
        }
    };

}
