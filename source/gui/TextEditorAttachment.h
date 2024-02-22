#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

namespace birdhouse
{

    class ITextEditorAttachment
    {
    public:
        virtual ~ITextEditorAttachment() = default;
        virtual void attach (const juce::String& parameterID) = 0;
    };

    template <typename ParamType>
    class TextEditorAttachment : public ITextEditorAttachment
    {
    public:
        TextEditorAttachment (juce::AudioProcessorValueTreeState& apvts, juce::TextEditor& editor, juce::UndoManager* undoManager = nullptr)
            : mApvts (apvts), mEditor (editor), mUndoManager (undoManager) {}

        void attach (const juce::String& parameterID) override
        {
            auto param = mApvts.getParameter (parameterID);

            if (param == nullptr)
            {
                DBG ("Parameter not found: " << parameterID);
                return;
            }

            auto updateEditor = [&] (float value) {
                if constexpr (std::is_same<ParamType, int>::value)
                {
                    auto intValue = static_cast<int> (value);
                    DBG ("Updating editor: " << intValue);
                    mEditor.setText (juce::String (intValue), juce::dontSendNotification);
                }
                else if constexpr (std::is_same<ParamType, float>::value)
                {
                    DBG ("Updating editor: " << value);
                    mEditor.setText (juce::String (value), juce::dontSendNotification);
                }
            };

            attachment = std::make_unique<juce::ParameterAttachment> (*param, updateEditor, mUndoManager);

            mEditor.onTextChange = [this, param] {
                if constexpr (std::is_same<ParamType, int>::value)
                {
                    auto value = mEditor.getText().getIntValue();
                    DBG ("Setting value: " << value);
                    param->setValueNotifyingHost (value);
                }
                else if constexpr (std::is_same<ParamType, float>::value)
                {
                    auto value = mEditor.getText().getFloatValue();
                    DBG ("Setting value: " << value);
                    param->setValueNotifyingHost (value);
                }
            };
        }

    private:
        juce::AudioProcessorValueTreeState& mApvts;
        juce::TextEditor& mEditor;
        juce::UndoManager* mUndoManager;
        std::unique_ptr<juce::ParameterAttachment> attachment;
    };

} // namespace birdhouse
