/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class MyPitchShiftAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Slider::Listener, public juce::ComboBox::Listener
{
public:
    MyPitchShiftAudioProcessorEditor (MyPitchShiftAudioProcessor&);
    ~MyPitchShiftAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    MyPitchShiftAudioProcessor& audioProcessor;

    juce::Slider mTranspoOneSlider;
    juce::Slider mTranspoTwoSlider;
    juce::Slider mTranspoThreeSlider;
    
    juce::Label mTranspoLabelOne;
    juce::Label mTranspoLabelTwo;
    juce::Label mTranspoLabelThree;
    
    juce::Slider mWindowSizeMs;
    
    juce::Label mWindowSizeLabel;
    
    juce::ComboBox mHarmPresetComboBox;
    
    enum HarmPreset
    {
        harm1 = 1,
        harm2 = 2,
        harm3 = 3,
        harm4 = 4,
    };

    void sliderValueChanged(juce::Slider* slider) override;
    void comboBoxChanged(juce::ComboBox* comboBox) override;
    
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MyPitchShiftAudioProcessorEditor)
};
