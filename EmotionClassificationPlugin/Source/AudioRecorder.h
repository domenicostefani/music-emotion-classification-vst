/**
 * Code for Audio Recorder VST
 * Adapted from Juce Demo App
 * Domenico Stefani 2023
*/
#pragma once

#include <string>
// #include "PluginProcessor.h"
#include <JuceHeader.h>

#ifndef MLOG_PATH
    #define MLOG_PATH "/tmp/"
#endif



class AudioRecorder {
public:

   #ifdef RECORDER_DEBUG_LOG
    AudioRecorder (std::unique_ptr<FileLogger> & mlogger) : logger(mlogger)
    {
       #ifdef RECORDER_DEBUG_LOG
        logText("> AudioRecorder()");
       #endif
        backgroundThread.startThread();
    }
   #else
    AudioRecorder ()
    {
        backgroundThread.startThread();
    }
   #endif

    ~AudioRecorder ()
    {
       #ifdef RECORDER_DEBUG_LOG
        logText("> ~AudioRecorder()");
       #endif
        stop();
    }

    void startRecording (const File& file, unsigned int numChannels)
    {
       #ifdef RECORDER_DEBUG_LOG
        logText("> startRecording()");
       #endif
        stop();

        if (sampleRate > 0)
        {
           #ifdef RECORDER_DEBUG_LOG
            logText("samplerate > 0");
           #endif
            // Create an OutputStream to write to our destination file...
            file.deleteFile();

            if (auto fileStream = std::unique_ptr<FileOutputStream> (file.createOutputStream()))
            {
               #ifdef RECORDER_DEBUG_LOG
                logText("filestream created");
               #endif
                // Now create a WAV writer object that writes to our output stream...
                WavAudioFormat wavFormat;
                
                //--- Discover possible bit depths ---//
                
                Array<int> possibleBitDepths = wavFormat.getPossibleBitDepths();
                std::string depthString = "Possible bit-depths: [";
                for(int dp = 0; dp < possibleBitDepths.size(); ++dp)
                {
                    depthString += std::to_string(possibleBitDepths[dp]);
                    depthString += " ";                
                }
                depthString += "]";
                
                bool contains24bits = possibleBitDepths.contains((int)24);

                
                //--- //////////////////////////// ---//

                if (auto writer = wavFormat.createWriterFor (fileStream.get(), sampleRate, numChannels, contains24bits ? 24 : 16, {}, 0))
                {
                   #ifdef RECORDER_DEBUG_LOG
                    logText("writer created");
                   #endif
                    fileStream.release(); // (passes responsibility for deleting the stream to the writer object that is now using it)

                    // Now we'll create one of these helper objects which will act as a FIFO buffer, and will
                    // write the data to disk on our background thread.
                    threadedWriter.reset (new AudioFormatWriter::ThreadedWriter (writer, backgroundThread, 32768));

                   #ifdef RECORDER_DEBUG_LOG
                    logText("threadedWriter reset");
                   #endif
                    // And now, swap over our active writer pointer so that the audio callback will start using it..
                    const ScopedLock sl (writerLock);

                   #ifdef RECORDER_DEBUG_LOG
                    logText("calling get on threaded writer");
                   #endif
                    activeWriter = threadedWriter.get();
                }
            }
        }
    }

    void stop()
    {

       #ifdef RECORDER_DEBUG_LOG
        logText("> stop()");
       #endif
        // First, clear this pointer to stop the audio callback from using our writer object..
        {
            const ScopedLock sl (writerLock);
            activeWriter = nullptr;
           #ifdef RECORDER_DEBUG_LOG
            logText("activewriter pointer cleaned");
           #endif
        }

        // Now we can delete the writer object. It's done in this order because the deletion could
        // take a little time while remaining data gets flushed to disk, so it's best to avoid blocking
        // the audio callback while this happens.
        threadedWriter.reset();

       #ifdef RECORDER_DEBUG_LOG
        logText("> threadedWriter reset");
       #endif
    }

    bool isRecording() const
    {
        return activeWriter.load() != nullptr;
    }

    void prepareToPlay(double sampleRate) //TODO: change and call from prepareToPlay
    {
       #ifdef RECORDER_DEBUG_LOG
        logText("> prepareToPlay()");
       #endif
        this->sampleRate = sampleRate;
    }

    void writeBlock(const float** inputChannelData, int numSamples)
    {
        const ScopedLock sl (writerLock);

        if (activeWriter.load() != nullptr)
        {
            activeWriter.load()->write (inputChannelData, numSamples);
        }
    }

private:
   #ifdef RECORDER_DEBUG_LOG
    std::unique_ptr<FileLogger> & logger;

    void logText(std::string text) const
    {
        if(logger)
            logger->logMessage(text);
    }
   #endif

    TimeSliceThread backgroundThread { "Audio Recorder Thread" }; // the thread that will write our audio data to disk
    std::unique_ptr<AudioFormatWriter::ThreadedWriter> threadedWriter; // the FIFO used to buffer the incoming data
    double sampleRate = 0.0;

    CriticalSection writerLock;
    std::atomic<AudioFormatWriter::ThreadedWriter*> activeWriter { nullptr };
};
