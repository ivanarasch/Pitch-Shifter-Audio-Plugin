/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
MyPitchShiftAudioProcessor::MyPitchShiftAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    for (int voice = 0; voice < NUM_VOICES; ++voice)
        mTranspo[voice] = 0.0;
    
    mWindowSizeMs = 50.0;
}

MyPitchShiftAudioProcessor::~MyPitchShiftAudioProcessor()
{
}

//==============================================================================
void MyPitchShiftAudioProcessor::setPhasorType(atec::LFO::LfoType t)
{
    for (int voice = 0; voice < NUM_VOICES; ++voice)
    {
        // make sure to set the Left and Right channel
        mPhasors[voice][0].setType(t);
        mPhasors[voice][1].setType(t);
    }
}

void MyPitchShiftAudioProcessor::setPhasorFreq(double f, int phasorIndex)
{
    mPhasors[phasorIndex][0].setFreq(f);
    mPhasors[phasorIndex][1].setFreq(f);
}

void MyPitchShiftAudioProcessor::setPhasorDebug(bool d)
{
    for (int voice = 0; voice < NUM_VOICES; ++voice)
    {
        mPhasors[voice][0].debug(d);
        mPhasors[voice][1].debug(d);
    }
}

void MyPitchShiftAudioProcessor::initPhasor()
{
    for (int voice = 0; voice < NUM_VOICES; ++voice)
    {
        mPhasors[voice][0].init();
        mPhasors[voice][1].init();
    }
}

const juce::String MyPitchShiftAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MyPitchShiftAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool MyPitchShiftAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool MyPitchShiftAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double MyPitchShiftAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MyPitchShiftAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int MyPitchShiftAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MyPitchShiftAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String MyPitchShiftAudioProcessor::getProgramName (int index)
{
    return {};
}

void MyPitchShiftAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void MyPitchShiftAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    mNumInputChannels = getTotalNumInputChannels();
    mBlockSize = samplesPerBlock;
    mSampleRate = sampleRate;

    // initialize mWindowSizeSamps now that we know the sampling rate
    mWindowSizeSamps = atec::Utilities::sec2samp(mWindowSizeMs / 1000.0, mSampleRate);

    mRingBuf.debug(false);
    // since our window size max is 300ms, the largest delay time we'll need is 0.3 * mSampleRate.
    // we'll bump that up to a second so there's more than enough space.
    mRingBuf.setSize(mNumInputChannels, 1.0 * mSampleRate, mBlockSize);
    mRingBuf.init();
    
    // turn on/off debug mode for all the phasor LFOs
    setPhasorDebug(false);
    // set the LFO type to saw for a 0-1 normalized phasor signal
    setPhasorType(atec::LFO::saw);
    // initialize phasor frequency with current mTranspo value
    for (int voice = 0; voice < NUM_VOICES; ++voice)
    {
        double phasorFreq = atec::Utilities::transpo2freq(mTranspo[voice], mWindowSizeMs);
        
        setPhasorFreq(phasorFreq, voice);
    }
    
    initPhasor();
}

void MyPitchShiftAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MyPitchShiftAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif


void MyPitchShiftAudioProcessor::computeDelayAndAmp(double phaseSample, double* envSignalPtr, double* delaySignalPtr)
{
    if (envSignalPtr && delaySignalPtr)
    {
        *envSignalPtr = std::sin (phaseSample * juce::MathConstants<double>::pi);
        *delaySignalPtr = phaseSample * mWindowSizeSamps;
    }
}


double MyPitchShiftAudioProcessor::computeTranspoSamples(int voice, int channel, int sample)
{
    double phasorSampleA, phasorSampleB;
    double envSignalA, envSignalB;
    double delayTimeSignalA, delayTimeSignalB;
    double sampleA, sampleB;
    
    // must call getNextSample() exactly once per sample
    phasorSampleA = mPhasors[voice][channel].getNextSample();
    // use the A reader's phasor and add 0.5, mod at 1.0 so that both phasor signals are guaranteed to be locked in a 180 degree out of phase relationship
    phasorSampleB = std::fmod (phasorSampleA + 0.5, 1.0);

    // phasor signal is in 0-1 range. it is multiplied by pi to become an angle for a sin function for amplitude enveloping (positive part of sin function), and by the window size in samples to become a delay time signal for steady change in delay time
    computeDelayAndAmp(phasorSampleA, &envSignalA, &delayTimeSignalA);

    // same for the B reader
    computeDelayAndAmp(phasorSampleB, &envSignalB, &delayTimeSignalB);
    // **** READER A
    //
    // get the interpolated sample
    // must offset the sample index i by -mWindowSizeSamps so that the delay ramping starts behind the RingBuffer write index by mWindowSizeSamps at a minimum
    sampleA = mRingBuf.readInterpSample (channel, sample - mWindowSizeSamps, delayTimeSignalA);
    // apply amplitude envelope
    sampleA *= envSignalA;


    // **** READER B
    //
    // get the interpolated sample
    sampleB = mRingBuf.readInterpSample (channel, sample - mWindowSizeSamps, delayTimeSignalB);
    // apply amplitude envelope
    sampleB *= envSignalB;
     
    // add the A and B signals together for output
    return sampleA + sampleB;
}

void MyPitchShiftAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    auto bufSize = buffer.getNumSamples();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // use a range-based for loop to look at the incoming MIDI messages
    // note that this will only work if you configure this project to accept MIDI input in Projucer
//    for (const auto metadata : midiMessages)
//    {
//        double phasorFreq;
//        auto message = metadata.getMessage();
//
//        if (message.isNoteOn())
//        {
//            DBG("MIDI Note: " + juce::String(message.getNoteNumber()));
//            mTranspo = message.getNoteNumber() - 60.0;
//            phasorFreq = atec::Utilities::transpo2freq(mTranspo, mWindowSizeMs);
//            setPhasorFreq(phasorFreq);
//        }
//    }
    
    // copy this block from the host into our ring buffer starting at mRingBufWriteIdx (both channels/all samples)
    mRingBuf.write(buffer);
    
    // now that we've buffered the incoming block, clear the buffer so we start with silence (both channels/all samples)
    buffer.clear();
    
    // pull a block of delayed interpolated audio from the RingBuffer
    // we'll read at two different positions A & B, and crossfade the results
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        
        for (int sample = 0; sample < bufSize; ++sample)
        {
            channelData[sample] = 0.0;
            
            for (int voice = 0; voice < NUM_VOICES; ++voice)
                channelData[sample] += computeTranspoSamples(voice, channel, sample);
        }
    }
    
    // the overlap-add can result in output with a greater amplitude than input, so we'll drop the gain by 3dB
    buffer.applyGain (juce::Decibels::decibelsToGain (-6.0));
    
}

//==============================================================================
bool MyPitchShiftAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* MyPitchShiftAudioProcessor::createEditor()
{
    return new MyPitchShiftAudioProcessorEditor (*this);
}

//==============================================================================
void MyPitchShiftAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void MyPitchShiftAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MyPitchShiftAudioProcessor();
}
