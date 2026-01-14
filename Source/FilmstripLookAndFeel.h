/*
  ==============================================================================

    FilmstripLookAndFeel.h
    Created: 17 Oct 2025 6:06:11pm
    Author:  John Wiens-Soshycki

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

// FilmstripLookAndFeel.h
struct FilmstripLookAndFeel : public juce::LookAndFeel_V4
{
    FilmstripLookAndFeel(juce::Image stripImage, int frameW, int frameH, int frames, bool vertical = true)
        : filmstrip(std::move(stripImage)),
          frameWidth(frameW), frameHeight(frameH),
          numFrames(frames), isVertical(vertical)
    {
        jassert (filmstrip.isValid());
        DBG("hasAlpha=" + std::to_string(filmstrip.hasAlphaChannel()));      // should be 1
        DBG("corner alpha=" + std::to_string((int) filmstrip.getPixelAt(1,1).getAlpha()));  // should be 0
        jassert (filmstrip.isValid());
        jassert (frameWidth  > 0 && frameHeight > 0 && numFrames > 0);
        if (isVertical)
            jassert (filmstrip.getWidth()  == frameWidth  && filmstrip.getHeight() == frameHeight * numFrames);
        else
            jassert (filmstrip.getHeight() == frameHeight && filmstrip.getWidth()  == frameWidth  * numFrames);
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h,
                          float sliderPosProportional, float /*rotaryStart*/, float /*rotaryEnd*/,
                          juce::Slider& slider) override
    {
        // Normalized [0..1] from the slider’s actual range:
        const auto t = juce::jlimit(0.0f, 1.0f,
            (float)((slider.getValue() - slider.getMinimum()) / (slider.getMaximum() - slider.getMinimum())));

        const int frameIndex = (int) std::round(t * (float)(numFrames - 1));

        // Destination is square-ish; center-crop to keep aspect ratio of frames
        const auto destW = (float) frameWidth;
        const auto destH = (float) frameHeight;
        const auto cx = (float) x + (float) w * 0.5f;
        const auto cy = (float) y + (float) h * 0.5f;
        const auto destX = cx - destW * 0.5f;
        const auto destY = cy - destH * 0.5f;
        
        //===========================
        //punchout fix
        /*
        g.saveState();
        juce::Path clip;
        clip.addEllipse (dest.reduced (0.5f)); // small reduce to avoid off-by-one edge pixels
        g.reduceClipRegion (clip);
       */

        if (isVertical)
        {
            const int srcY = frameIndex * frameHeight;
            g.drawImage(filmstrip,
                        (int)destX, (int)destY, (int)destW, (int)destH,     // destination
                        0, srcY, frameWidth, frameHeight, false);                  // source
        }
        else
        {
            const int srcX = frameIndex * frameWidth;
            g.drawImage(filmstrip,
                        (int)destX, (int)destY, (int)destW, (int)destH,
                        srcX, 0, frameWidth, frameHeight, false);
        }
    }

    void drawLinearSlider(juce::Graphics& g, int x, int y, int w, int h,
                          float sliderPos, float /*minPos*/, float /*maxPos*/,
                          const juce::Slider::SliderStyle, juce::Slider& slider) override
    {
        // Same mapping, but you may prefer a horizontal filmstrip for linear sliders.
        const auto t = juce::jlimit(0.0f, 1.0f,
            (float)((slider.getValue() - slider.getMinimum()) / (slider.getMaximum() - slider.getMinimum())));
        const int frameIndex = (int) std::round(t * (float)(numFrames - 1));

        const auto destW = (float) frameWidth;
        const auto destH = (float) frameHeight;
        const auto destX = (float) x + ((float) w - destW) * 0.5f;
        const auto destY = (float) y + ((float) h - destH) * 0.5f;

        if (isVertical)
        {
            const int srcY = frameIndex * frameHeight;
            g.drawImage(filmstrip,
                        (int)destX, (int)destY, (int)destW, (int)destH,     // destination
                        0, srcY, frameWidth, frameHeight);                  // source
        }
        else
        {
            const int srcX = frameIndex * frameWidth;
            g.drawImage(filmstrip,
                        (int)destX, (int)destY, (int)destW, (int)destH,
                        srcX, 0, frameWidth, frameHeight);
        }
    }

private:
    juce::Image filmstrip;
    int frameWidth = 0, frameHeight = 0, numFrames = 0;
    bool isVertical = true;
};

