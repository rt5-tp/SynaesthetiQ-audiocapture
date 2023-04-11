#ifndef FFT_PROCESSOR_H
#define FFT_PROCESSOR_H

#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <fftw3.h>
#include <cmath>

enum ReductionMethod {
    AVERAGE,
    MAXIMUM
};

class FFTProcessor {
public:
    using DataAvailableCallback = void(*)(const std::vector<double> &);

    FFTProcessor();
    ~FFTProcessor();

    void start();
    void stop();
    static void audio_callback(const std::vector<short> &data);

    void registerCallback(DataAvailableCallback cb);

    ReductionMethod reductionMethod = AVERAGE;
    std::vector<double> getReducedResolutionFFT(const std::vector<double>& fftOutputData, int numSections);

private:
    void workerThread();
    void performFFT(const std::vector<double> &data);

    static FFTProcessor* singleton;

    std::thread fftThread;
    std::mutex mtx;
    std::condition_variable audio_available;
    static std::condition_variable saudio_available;
    bool stopThread;
    bool newData;
    std::vector<double> inputData;
    DataAvailableCallback callback;
};

#endif // FFT_PROCESSOR_H
