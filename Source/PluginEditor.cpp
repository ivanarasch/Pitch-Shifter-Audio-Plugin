/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
MyPitchShiftAudioProcessorEditor::MyPitchShiftAudioProcessorEditor (MyPitchShiftAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (600, 350);

    mTranspoOneSlider.setTextBoxStyle(juce::Slider::TextBoxAbove, false, 100, 25);
    mTranspoOneSlider.setRange(-12.0, 12.0, 0.01); // step in hundreths of a semi-tone
    mTranspoOneSlider.setValue(audioProcessor.mTranspo[0]);
    addAndMakeVisible(&mTranspoOneSlider);
    mTranspoOneSlider.addListener(this);
    
    mTranspoTwoSlider.setTextBoxStyle(juce::Slider::TextBoxAbove, false, 100, 25);
    mTranspoTwoSlider.setRange(-12.0, 12.0, 0.01); // step in hundreths of a semi-tone
    mTranspoTwoSlider.setValue(audioProcessor.mTranspo[1]);
    addAndMakeVisible(&mTranspoTwoSlider);
    mTranspoTwoSlider.addListener(this);
    
    mTranspoThreeSlider.setTextBoxStyle(juce::Slider::TextBoxAbove, false, 100, 25);
    mTranspoThreeSlider.setRange(-12.0, 12.0, 0.01); // step in hundreths of a semi-tone
    mTranspoThreeSlider.setValue(audioProcessor.mTranspo[2]);
    addAndMakeVisible(&mTranspoThreeSlider);
    mTranspoThreeSlider.addListener(this);

    mWindowSizeMs.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 100, 25);
    mWindowSizeMs.setRange(5.0, 300.0, 0.1);
    mWindowSizeMs.setValue(audioProcessor.mWindowSizeMs);
    addAndMakeVisible(&mWindowSizeMs);
    mWindowSizeMs.addListener(this);
    
    addAndMakeVisible (&mTranspoLabelOne);
    mTranspoLabelOne.setText ("Transposition Voice 1", juce::dontSendNotification);
    mTranspoLabelOne.attachToComponent (&mTranspoOneSlider, true);
    mTranspoLabelOne.setColour (juce::Label::textColourId, juce::Colours::black);
    mTranspoLabelOne.setJustificationType (juce::Justification::bottomLeft);
    
    addAndMakeVisible (&mTranspoLabelTwo);
    mTranspoLabelTwo.setText ("Transposition Voice 2", juce::dontSendNotification);
    mTranspoLabelTwo.attachToComponent (&mTranspoTwoSlider, true);
    mTranspoLabelTwo.setColour (juce::Label::textColourId, juce::Colours::black);
    mTranspoLabelTwo.setJustificationType (juce::Justification::bottomLeft);
    
    addAndMakeVisible (&mTranspoLabelThree);
    mTranspoLabelThree.setText ("Transposition Voice 3", juce::dontSendNotification);
    mTranspoLabelThree.attachToComponent (&mTranspoThreeSlider, true);
    mTranspoLabelThree.setColour (juce::Label::textColourId, juce::Colours::black);
    mTranspoLabelThree.setJustificationType (juce::Justification::bottomLeft);

    addAndMakeVisible (&mWindowSizeLabel);
    mWindowSizeLabel.setText ("Window Size (ms)", juce::dontSendNotification);
    mWindowSizeLabel.attachToComponent (&mWindowSizeMs, true);
    mWindowSizeLabel.setColour (juce::Label::textColourId, juce::Colours::black);
    mWindowSizeLabel.setJustificationType (juce::Justification::topLeft);
    
    mHarmPresetComboBox.addItem("Minor 3rd",harm1 );
    mHarmPresetComboBox.addItem("Major 3rd",harm2 );
    mHarmPresetComboBox.addItem("Major 7th", harm3);
    mHarmPresetComboBox.addItem("Perfect Fifth",harm4 );
    addAndMakeVisible(&mHarmPresetComboBox);
    mHarmPresetComboBox.addListener(this);
    
    
}

MyPitchShiftAudioProcessorEditor::~MyPitchShiftAudioProcessorEditor()
{
    mTranspoOneSlider.removeListener(this);
    mTranspoTwoSlider.removeListener(this);
    mTranspoThreeSlider.removeListener(this);
    
    mWindowSizeMs.removeListener(this);
}

//==============================================================================
void MyPitchShiftAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &mWindowSizeMs)
    {
        double windowSizeSec;

        audioProcessor.mWindowSizeMs = mWindowSizeMs.getValue();
        windowSizeSec = audioProcessor.mWindowSizeMs / 1000.0;
        audioProcessor.mWindowSizeSamps = atec::Utilities::sec2samp(windowSizeSec, audioProcessor.mSampleRate);

        DBG("Window ms: " + juce::String(audioProcessor.mWindowSizeMs));
        DBG("Window samples: " + juce::String(audioProcessor.mWindowSizeSamps));
    }
    
    if (slider == &mTranspoOneSlider)
    {
        double phasorFreq;

        audioProcessor.mTranspo[0] = mTranspoOneSlider.getValue();

        phasorFreq = atec::Utilities::transpo2freq(audioProcessor.mTranspo[0], audioProcessor.mWindowSizeMs);
        audioProcessor.setPhasorFreq(phasorFreq, 0);

        DBG("Transpo: " + juce::String(audioProcessor.mTranspo[0]));
        DBG("Freq: " + juce::String(phasorFreq));
    }

    if (slider == &mTranspoTwoSlider)
    {
        double phasorFreq;

        audioProcessor.mTranspo[1] = mTranspoOneSlider.getValue();

        phasorFreq = atec::Utilities::transpo2freq(audioProcessor.mTranspo[1], audioProcessor.mWindowSizeMs);
        audioProcessor.setPhasorFreq(phasorFreq, 1);

        DBG("Transpo: " + juce::String(audioProcessor.mTranspo[1]));
        DBG("Freq: " + juce::String(phasorFreq));
    }

    if (slider == &mTranspoThreeSlider)
    {
        double phasorFreq;

        audioProcessor.mTranspo[2] = mTranspoOneSlider.getValue();

        phasorFreq = atec::Utilities::transpo2freq(audioProcessor.mTranspo[2], audioProcessor.mWindowSizeMs);
        audioProcessor.setPhasorFreq(phasorFreq, 2);

        DBG("Transpo: " + juce::String(audioProcessor.mTranspo[2]));
        DBG("Freq: " + juce::String(phasorFreq));
    }
}
void MyPitchShiftAudioProcessorEditor::comboBoxChanged(juce::ComboBox* comboBox)
{
    switch (mHarmPresetComboBox.getSelectedId())
    {
        case harm1:
            mTranspoOneSlider.setValue(0.0);
            mTranspoTwoSlider.setValue(3.0);
            mTranspoThreeSlider.setValue(7.0);
            break;
        case harm2:
            mTranspoOneSlider.setValue(0.0);
            mTranspoTwoSlider.setValue(4.0);
            mTranspoThreeSlider.setValue(7.0);
            break;
        case harm3:
            mTranspoOneSlider.setValue(0.0);
            mTranspoTwoSlider.setValue(4.0);
            mTranspoThreeSlider.setValue(11.0);
            break;
        case harm4:
            mTranspoOneSlider.setValue(0.0);
            mTranspoTwoSlider.setValue(7.0);
            mTranspoThreeSlider.setValue(12.0);
            break;
    
        default:
            break;
    }
}

void MyPitchShiftAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (juce::Colours::slategrey);
}

void MyPitchShiftAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    mTranspoOneSlider.setBounds(150, 50, 300, 50);
    mTranspoTwoSlider.setBounds(150,100,300,50);
    mTranspoThreeSlider.setBounds(150,150,300,50);
    mWindowSizeMs.setBounds(150,250, 300, 50);
    mHarmPresetComboBox.setBounds(390,290,50,50);
    
}
