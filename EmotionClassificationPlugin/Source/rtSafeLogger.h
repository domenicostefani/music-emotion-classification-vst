/*
  ==============================================================================

    Real time logger for timbreID

    Logger that works with real time threads
    It makes use of a circular buffer to post some messages and have them
    written to file by a polling routine outside of the real time audio thread.

    Created: 19 Nov 2020
    Author:  Domenico Stefani (domenico.stefani[at]unitn.it)

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include <cstring>  // For strncpy
#include <memory>   // For unique_ptr

#include "containers/choc_SingleReaderSingleWriterFIFO.h"  // Jules' thread safe fifo buffer implementation

// #define DISABLE_LOG // This disables logging, and calls functions with empty bodies
// #define UNSAFE_QUICK_LOG // This makes the logger write directly to file, breaking the real-time safety to quickly debug the code

typedef std::function<void(void)> TimerEventCallback;

namespace rtutils {

/** Logger class for real-time usage */
class RealTimeLogger {
public:
    static const size_t LOG_MAX_ENTRIES = 2048;  // Size of the fifo buffer for the log

    /** Entry in the log */
    struct LogEntry {
        static const int MESSAGE_LENGTH = 240;  // Length of messages in the fifo buffer
        int64 timeAtStart,                      // Intended to log time intervals
            timeAtEnd;                          // Intended to log time intervals
        char message[MESSAGE_LENGTH + 1];
    };

    RealTimeLogger(std::string logname, std::string logdir = "/tmp/", size_t writePeriodMs = 100) {
#ifndef DISABLE_LOG
        this->name = logname;
        this->logdir = logdir;
        std::replace(logname.begin(), logname.end(), ' ', '_');
        if (!this->fileLogger)
            this->fileLogger = std::unique_ptr<FileLogger>(FileLogger::createDateStampedLogger(logdir, logname, ".log", logname));
    #ifndef UNSAFE_QUICK_LOG
        fifoBuffer.reset(LOG_MAX_ENTRIES);
        pollingTimer.startLogRoutine(writePeriodMs);  // Call every writePeriodMs milliseconds, defaults to 0.1 seconds
    #endif
#endif
    }
    ~RealTimeLogger(){};

    bool logInfo(const char message[], int64 timeAtStart, int64 timeAtEnd);
    bool logInfo(const char message[], const char suffix[] = nullptr);

    bool logValue(const char message[], int value, const char suffix[] = nullptr);
    bool logValue(const char message[], float value, const char suffix[] = nullptr);
    bool logValue(const char message[], double value, const char suffix[] = nullptr);
    bool logValue(const char message[], long unsigned int value, const char suffix[] = nullptr);
    bool logValue(const char message[], unsigned int value, const char suffix[] = nullptr);
    bool logValue(const char message[], const char value[]);
#ifndef UNSAFE_QUICK_LOG
    bool pop(LogEntry& logEntry);
#endif

    std::string getName() const { return this->name; }

private:
    const int64 highResFrequency = juce::Time::getHighResolutionTicksPerSecond();
    std::string name = "", logdir = "";
    std::unique_ptr<FileLogger> fileLogger;
#ifndef UNSAFE_QUICK_LOG
    choc::fifo::SingleReaderSingleWriterFIFO<RealTimeLogger::LogEntry> fifoBuffer;

    /** POLLING ROUTINE (called at regular intervals by the timer) **/
    void logPollingRoutine() {
        LogEntry le;
        while (this->pop(le))  // Continue if there are log entries in the rtlogger queue
        {
            /** CONSUME RTLOGGER ENTRIES AND WRITE THEM TO FILE **/
            std::string msg = "";
            msg += le.message;
            double start, end, diff;
            start = (le.timeAtStart * 1.0f) / this->highResFrequency;
            end = (le.timeAtEnd * 1.0f) / this->highResFrequency;
            diff = end - start;
            if (start != 0.0f) {
                msg += " | start: " + std::to_string(start);
                msg += " | end: " + std::to_string(end);
                msg += " | difference: " + std::to_string(diff) + "s";
            }
            fileLogger->logMessage(msg);
        }
    }

    class PollingTimer : public Timer {
    public:
        PollingTimer(TimerEventCallback cb) {
            this->cb = cb;
        }
        void startLogRoutine() {
            Timer::startTimer(this->interval);
        }
        void startLogRoutine(int interval) {
            this->interval = interval;
            Timer::startTimer(this->interval);
        }
        void timerCallback() override {
            cb();
        }

    private:
        TimerEventCallback cb;
        int interval = 500;  // Half second default polling interval
    };

    PollingTimer pollingTimer{[this] { logPollingRoutine(); }};
#endif
};

/** Add message to the log buffer */
inline bool RealTimeLogger::logInfo(const char message[], int64 timeAtStart, int64 timeAtEnd) {
#ifdef UNSAFE_QUICK_LOG
    std::string msg = "";
    msg += message;
    double start, end, diff;
    start = (timeAtStart * 1.0f) / this->highResFrequency;
    end = (timeAtEnd * 1.0f) / this->highResFrequency;
    diff = end - start;
    if (start != 0.0f) {
        msg += " | start: " + std::to_string(start);
        msg += " | end: " + std::to_string(end);
        msg += " | difference: " + std::to_string(diff) + "s";
    }
    fileLogger->logMessage(msg);
    return true;
#else
    #ifndef DISABLE_LOG
    LogEntry logEntry;
    logEntry.timeAtStart = timeAtStart;
    logEntry.timeAtEnd = timeAtEnd;
    strncpy(logEntry.message, message, LogEntry::MESSAGE_LENGTH);
    logEntry.message[LogEntry::MESSAGE_LENGTH] = '\0';
    return fifoBuffer.push(logEntry);
    #else
    return true;
    #endif
#endif
}

/** Add message to the log buffer */
inline bool RealTimeLogger::logInfo(const char message[], const char suffix[]) {
#ifdef UNSAFE_QUICK_LOG
    std::string msg = "";
    msg += message;
    if (suffix) {
        msg += " ";
        msg += suffix;
    }
    fileLogger->logMessage(msg);
    return true;
#else
    #ifndef DISABLE_LOG
    if (suffix) {
        char cmessage[LogEntry::MESSAGE_LENGTH + 1];
        snprintf(cmessage, sizeof(cmessage), "%s %s", message, suffix);
        return logInfo(cmessage, 0, 0);
    } else {
        return logInfo(message, 0, 0);
    }
    #else
    return true;
    #endif
#endif
}

#ifndef UNSAFE_QUICK_LOG
/** Pop safely an element from the fifo buffer
 *  Do this from a non RT thread if you intend on writing to a file
 */
inline bool RealTimeLogger::pop(LogEntry& logEntry) {
    return fifoBuffer.pop(logEntry);
}
#endif

inline bool RealTimeLogger::logValue(const char message[], int value, const char suffix[]) {
#ifdef UNSAFE_QUICK_LOG
    std::string msg = "";
    msg += message;
    msg += " ";
    msg += std::to_string(value);
    if (suffix) {
        msg += " ";
        msg += suffix;
    }
    fileLogger->logMessage(msg);
    return true;
#else
    #ifndef DISABLE_LOG
    char cmessage[LogEntry::MESSAGE_LENGTH + 1];
    if (suffix)
        snprintf(cmessage, sizeof(cmessage), "%s %d %s", message, value, suffix);
    else
        snprintf(cmessage, sizeof(cmessage), "%s %d", message, value);
    return logInfo(cmessage);
    #else
    return true;
    #endif
#endif
}

inline bool RealTimeLogger::logValue(const char message[], float value, const char suffix[]) {
#ifdef UNSAFE_QUICK_LOG
    std::string msg = "";
    msg += message;
    msg += " ";
    msg += std::to_string(value);
    if (suffix) {
        msg += " ";
        msg += suffix;
    }
    fileLogger->logMessage(msg);
    return true;
#else
    #ifndef DISABLE_LOG
    char cmessage[LogEntry::MESSAGE_LENGTH + 1];
    if (suffix)
        snprintf(cmessage, sizeof(cmessage), "%s %f %s", message, value, suffix);
    else
        snprintf(cmessage, sizeof(cmessage), "%s %f", message, value);
    return logInfo(cmessage);
    #else
    return true;
    #endif
#endif
}

inline bool RealTimeLogger::logValue(const char message[], double value, const char suffix[]) {
#ifdef UNSAFE_QUICK_LOG
    std::string msg = "";
    msg += message;
    msg += " ";
    msg += std::to_string(value);
    if (suffix) {
        msg += " ";
        msg += suffix;
    }
    fileLogger->logMessage(msg);
    return true;
#else
    #ifndef DISABLE_LOG
    char cmessage[LogEntry::MESSAGE_LENGTH + 1];
    if (suffix)
        snprintf(cmessage, sizeof(cmessage), "%s %f %s", message, value, suffix);
    else
        snprintf(cmessage, sizeof(cmessage), "%s %f", message, value);
    return logInfo(cmessage);
    #else
    return true;
    #endif
#endif
}

inline bool RealTimeLogger::logValue(const char message[], long unsigned int value, const char suffix[]) {
#ifdef UNSAFE_QUICK_LOG
    std::string msg = "";
    msg += message;
    msg += " ";
    msg += std::to_string(value);
    if (suffix) {
        msg += " ";
        msg += suffix;
    }
    fileLogger->logMessage(msg);
    return true;
#else
    #ifndef DISABLE_LOG
    char cmessage[LogEntry::MESSAGE_LENGTH + 1];
    if (suffix)
        snprintf(cmessage, sizeof(cmessage), "%s %lu %s", message, value, suffix);
    else
        snprintf(cmessage, sizeof(cmessage), "%s %lu", message, value);
    return logInfo(cmessage);
    #else
    return true;
    #endif
#endif
}

inline bool RealTimeLogger::logValue(const char message[], unsigned int value, const char suffix[]) {
#ifdef UNSAFE_QUICK_LOG
    std::string msg = "";
    msg += message;
    msg += " ";
    msg += std::to_string(value);
    if (suffix) {
        msg += " ";
        msg += suffix;
    }
    fileLogger->logMessage(msg);
    return true;
#else
    #ifndef DISABLE_LOG
    char cmessage[LogEntry::MESSAGE_LENGTH + 1];
    if (suffix)
        snprintf(cmessage, sizeof(cmessage), "%s %u %s", message, value, suffix);
    else
        snprintf(cmessage, sizeof(cmessage), "%s %u", message, value);
    return logInfo(cmessage);
    #else
    return true;
    #endif
#endif
}

inline bool RealTimeLogger::logValue(const char message[], const char value[]) {
#ifdef UNSAFE_QUICK_LOG
    std::string msg = "";
    msg += message;
    msg += " ";
    msg += value;
    fileLogger->logMessage(msg);
    return true;
#else
    #ifndef DISABLE_LOG
    return this->logInfo(message, value);
    #else
    return true;
    #endif
#endif
}

}  // namespace rtutils
