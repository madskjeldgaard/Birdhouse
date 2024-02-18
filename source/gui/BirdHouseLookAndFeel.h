#pragma once

#include "BinaryData.h"
#include <juce_gui_basics/juce_gui_basics.h>

namespace BirdHouse
{

    // Colours
    namespace Colours
    {
        // Define the color palette based on the Tokyo Night theme
        const static auto bgDark = juce::Colour::fromRGB (0x1f, 0x23, 0x35);
        const static auto bg = juce::Colour::fromRGB (0x24, 0x28, 0x3b);
        const static auto fg = juce::Colour::fromRGB (0xc0, 0xca, 0xf5);
        const static auto comment = juce::Colour::fromRGB (0x56, 0x5f, 0x89);
        const static auto blue = juce::Colour::fromRGB (0x7a, 0xa2, 0xf7);
        const static auto magenta = juce::Colour::fromRGB (0xbb, 0x9a, 0xf7);
        const static auto green = juce::Colour::fromRGB (0x9e, 0xce, 0x6a);
        const static auto red = juce::Colour::fromRGB (0xf7, 0x76, 0x8e);
        const static auto yellow = juce::Colour::fromRGB (0xe0, 0xaf, 0x68);
    }

    class BirdHouseLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        BirdHouseLookAndFeel()
        {
            // Set default colours for components
            setColour (juce::Slider::backgroundColourId, Colours::bgDark);
            setColour (juce::Slider::thumbColourId, Colours::blue);
            setColour (juce::Slider::trackColourId, Colours::fg);
            setColour (juce::Label::textColourId, Colours::fg);

            // setColour (juce::TextButton::buttonColourId, Colours::bgDark);
            setColour (juce::TextButton::buttonColourId, Colours::bgDark);
            setColour (juce::TextButton::buttonOnColourId, Colours::yellow);
            setColour (juce::TextButton::textColourOnId, Colours::fg);

            setColour (juce::ComboBox::backgroundColourId, Colours::bg);
            setColour (juce::ComboBox::textColourId, Colours::fg);
            setColour (juce::ComboBox::outlineColourId, Colours::fg.withAlpha (0.5f));
            setColour (juce::PopupMenu::backgroundColourId, Colours::bgDark);
            setColour (juce::PopupMenu::textColourId, Colours::blue);
            setColour (juce::ResizableWindow::backgroundColourId, Colours::bg);
            setColour (juce::TextEditor::backgroundColourId, Colours::bgDark);
            setColour (juce::TextEditor::textColourId, Colours::fg);
            setColour (juce::TextEditor::highlightColourId, Colours::blue.withAlpha (0.5f));
            setColour (juce::TextEditor::highlightedTextColourId, Colours::magenta);
            setColour (juce::TextEditor::outlineColourId, Colours::fg.withAlpha (0.5f));
            setColour (juce::TextEditor::focusedOutlineColourId, Colours::magenta);

            // Additional component colours can be set here following the same pattern
        }

        // Override other methods to use the custom font where necessary

        juce::Font getComboBoxFont (juce::ComboBox& box) override
        {
            // Hack combo box font size
            return getCustomFont().withHeight (box.getHeight() / 2);
        }

        juce::Font getPopupMenuFont() override
        {
            return getCustomFont();
        }

        juce::Font getTextButtonFont (juce::TextButton&, int buttonHeight) override
        {
            return getCustomFont().withHeight (buttonHeight / 2);
        }

        void drawButtonBackground (juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
        {
            auto cornerSize = 0.0f;
            auto lineThickness = 1.0f;
            auto bounds = button.getLocalBounds().toFloat().reduced (0.5f, 0.5f);

            auto baseColour = backgroundColour.withMultipliedSaturation (button.hasKeyboardFocus (true) ? 1.3f : 0.9f)
                                  .withMultipliedAlpha (button.isEnabled() ? 1.0f : 0.5f);

            if (shouldDrawButtonAsDown || shouldDrawButtonAsHighlighted)
                baseColour = baseColour.contrasting (shouldDrawButtonAsDown ? 0.2f : 0.05f);

            g.setColour (baseColour);

            auto flatOnLeft = button.isConnectedOnLeft();
            auto flatOnRight = button.isConnectedOnRight();
            auto flatOnTop = button.isConnectedOnTop();
            auto flatOnBottom = button.isConnectedOnBottom();

            if (flatOnLeft || flatOnRight || flatOnTop || flatOnBottom)
            {
                juce::Path path;
                path.addRoundedRectangle (bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight(), cornerSize, cornerSize, !(flatOnLeft || flatOnTop), !(flatOnRight || flatOnTop), !(flatOnLeft || flatOnBottom), !(flatOnRight || flatOnBottom));

                g.fillPath (path);

                g.setColour (button.findColour (juce::ComboBox::outlineColourId));
                g.strokePath (path, juce::PathStrokeType (1.0f));
            }
            else
            {
                g.fillRoundedRectangle (bounds, cornerSize);

                g.setColour (button.findColour (juce::ComboBox::outlineColourId));
                g.drawRoundedRectangle (bounds, cornerSize, lineThickness);
            }
        }

        void drawComboBox (juce::Graphics& g, int width, int height, bool, int, int, int, int, juce::ComboBox& box) override
        {
            // auto cornerSize = box.findParentComponentOfClass<juce::ChoicePropertyComponent>() != nullptr ? 0.0f : 3.0f;
            auto cornerSize = 0.f;
            juce::Rectangle<int> boxBounds (0, 0, width, height);

            g.setColour (box.findColour (juce::ComboBox::backgroundColourId));
            g.fillRoundedRectangle (boxBounds.toFloat(), cornerSize);

            g.setColour (box.findColour (juce::ComboBox::outlineColourId));
            g.drawRoundedRectangle (boxBounds.toFloat().reduced (0.5f, 0.5f), cornerSize, 1.0f);

            juce::Rectangle<int> arrowZone (width - 30, 0, 20, height);
            juce::Path path;
            path.startNewSubPath ((float) arrowZone.getX() + 3.0f, (float) arrowZone.getCentreY() - 2.0f);
            path.lineTo ((float) arrowZone.getCentreX(), (float) arrowZone.getCentreY() + 3.0f);
            path.lineTo ((float) arrowZone.getRight() - 3.0f, (float) arrowZone.getCentreY() - 2.0f);

            g.setColour (box.findColour (juce::ComboBox::arrowColourId).withAlpha ((box.isEnabled() ? 0.9f : 0.2f)));
            g.strokePath (path, juce::PathStrokeType (2.0f));
        }

    private:
        juce::Font
            getCustomFont()
        {
            static auto typeface = juce::Typeface::createSystemTypefaceFor (BinaryData::JetBrainsMonoRegular_ttf, BinaryData::JetBrainsMonoRegular_ttfSize);
            return juce::Font (typeface);
        }

        juce::Typeface::Ptr getTypefaceForFont (const juce::Font& f) override
        {
            // Check for JetBrains Mono bold
            if (f.isBold() && !f.isItalic())
            {
                return juce::Typeface::createSystemTypefaceFor (BinaryData::JetBrainsMonoBold_ttf, BinaryData::JetBrainsMonoBold_ttfSize);
            }
            // Check for JetBrains Mono italic
            else if (!f.isBold() && f.isItalic())
            {
                return juce::Typeface::createSystemTypefaceFor (BinaryData::JetBrainsMonoItalic_ttf, BinaryData::JetBrainsMonoItalic_ttfSize);
            }
            // Check for JetBrains Mono bold italic
            else if (f.isBold() && f.isItalic())
            {
                // Assuming you have a bold italic font file
                return juce::Typeface::createSystemTypefaceFor (BinaryData::JetBrainsMonoBoldItalic_ttf, BinaryData::JetBrainsMonoBoldItalic_ttfSize);
            }
            // Fallback to regular JetBrains Mono
            else
            {
                return juce::Typeface::createSystemTypefaceFor (BinaryData::JetBrainsMonoRegular_ttf, BinaryData::JetBrainsMonoRegular_ttfSize);
            }
        }
    };
}
