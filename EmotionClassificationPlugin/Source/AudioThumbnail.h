/**
 * @file AudioThumbnail.h
 * @author Domenico Stefani
 * @brief Audio Thumbnail from the JUCE tutorial with added sectioning and
 * labels for each section
 * @version 0.1
 * @date 2023-05-08
 *
 *
 */

#pragma once

//==============================================================================
class AudioThumbnailComponent : public juce::Component,
                                private juce::ChangeListener {
public:
    AudioThumbnailComponent(std::string nofiletext = "No File Loaded")
        : thumbnailCache(5),
          thumbnail(512, formatManager, thumbnailCache),
          nofileText(nofiletext) {
        formatManager.registerBasicFormats();
        thumbnail.addChangeListener(this);
    }

    ~AudioThumbnailComponent() override {}

    void paint(juce::Graphics &g) override {
        auto backcolor = getLookAndFeel().findColour(
            juce::ResizableWindow::backgroundColourId);
        g.setColour(backcolor);
        g.setColour(juce::Colours::white);

        auto area = getLocalBounds();
        auto rectbox = area.reduced(10);
        Path p;
        p.addRectangle(rectbox.getX(), rectbox.getY(), rectbox.getWidth(),
                       rectbox.getHeight());
        g.strokePath(p, PathStrokeType(1.f));
        // g.fillRect (rectbox);

        rectbox = rectbox.reduced(5);
        rectbox.removeFromBottom(25);
        juce::Rectangle<int> thumbnailBounds = rectbox;

        if (thumbnail.getNumChannels() == 0)
            paintIfNoFileLoaded(g, thumbnailBounds);
        else
            paintIfFileLoaded(g, thumbnailBounds);
    }

    void changeListenerCallback(juce::ChangeBroadcaster *source) override {
        if (source == &thumbnail) thumbnailChanged();
    }

    void setFileSource(juce::File file, bool resetLabels = true) {
        if (resetLabels) {
            setSectionLabels({});
        }

        if (file != juce::File{}) {
            auto *reader = formatManager.createReaderFor(file);

            if (reader != nullptr) {
                auto newSource =
                    std::make_unique<juce::AudioFormatReaderSource>(reader,
                                                                    true);
                // transportSource.setSource (newSource.get(), 0, nullptr,
                // reader->sampleRate);
                thumbnail.setSource(new juce::FileInputSource(file));
                // readerSource.reset (newSource.release());
            }
        }
    }

    void setSectionLabels(const std::vector<std::tuple<float, float, std::string>> &labels,
                          const std::vector<juce::Colour> &colors = {}) {
        sectionLabels = labels;
        sectionLabelColors = colors;
        repaint();
    }

private:
    void thumbnailChanged() { repaint(); }

    void paintIfNoFileLoaded(juce::Graphics &g,
                             const juce::Rectangle<int> &thumbnailBounds) {
        auto backcolor = getLookAndFeel().findColour(
            juce::ResizableWindow::backgroundColourId);
        g.setColour(backcolor);
        g.fillRect(thumbnailBounds);
        g.setColour(juce::Colours::white);
        g.drawFittedText(nofileText, thumbnailBounds,
                         juce::Justification::centred, 1);
    }

    void drawVline(juce::Graphics &g, float lineX, float lineY, float lineH,
                   float lineThickness = 1.f,
                   juce::Colour color = juce::Colours::darkred) {
        g.setColour(color);
        Path p;
        float lineW = 1.0f;
        p.addRectangle(lineX, lineY, lineW, lineH);
        g.strokePath(p, PathStrokeType(lineThickness));
    }

    void paintIfFileLoaded(juce::Graphics &g,
                           const juce::Rectangle<int> &thumbnailBounds) {
        auto backcolor = getLookAndFeel().findColour(
            juce::ResizableWindow::backgroundColourId);
        g.setColour(backcolor);
        // Draw outer Rectangle
        g.fillRect(thumbnailBounds);
        // Draw waveform (audio thumbnail)
        g.setColour(juce::Colours::white);
        thumbnail.drawChannels(g, thumbnailBounds,
                               0.0,                         // start time
                               thumbnail.getTotalLength(),  // end time
                               1.0f);                       // vertical zoom

        auto thumbNailLength = thumbnail.getTotalLength();
        auto thumbNailHeight = thumbnailBounds.getHeight();
        auto thumbNailWidth = thumbnailBounds.getWidth();
        auto thumbNailX = thumbnailBounds.getX();
        auto thumbNailY = thumbnailBounds.getY();

        float magnification = thumbNailWidth / thumbNailLength;  // Ratio between the length of the
                                                                 // representation in pixels and in seconds
        if (sectionLabels.size() > 0) {
            for (int i = 0; i < sectionLabels.size(); i++) {
                float sectionStart = std::get<0>(sectionLabels[i]);
                float sectionEnd = std::get<1>(sectionLabels[i]);
                std::string sectionLabel = std::get<2>(sectionLabels[i]);

                drawVline(g, sectionStart * magnification + thumbNailX, thumbNailY, thumbNailHeight, 1.f, juce::Colours::red);
                drawVline(g, sectionEnd   * magnification + thumbNailX, thumbNailY, thumbNailHeight, 1.f, juce::Colours::red);

                g.setColour(sectionLabelColors.size() == sectionLabels.size() ? sectionLabelColors[i] : juce::Colours::white);
                g.setFont(juce::Font(15.f, juce::Font::bold));
                g.drawFittedText(
                    sectionLabel,
                    sectionStart * magnification + thumbNailX,
                    thumbNailY + thumbNailHeight,
                    (sectionEnd-sectionStart) * magnification, 20,
                    juce::Justification::centred, 2);
            }
        }
    }

    juce::AudioFormatManager formatManager;
    juce::AudioThumbnailCache thumbnailCache;
    juce::AudioThumbnail thumbnail;

    std::vector<std::tuple<float, float, std::string>> sectionLabels;
    std::vector<juce::Colour> sectionLabelColors;

    std::string nofileText;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioThumbnailComponent)
};
