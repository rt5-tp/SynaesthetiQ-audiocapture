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
    void processData(const std::vector<short> &data);
    void registerCallback(DataAvailableCallback cb);

    ReductionMethod reductionMethod = AVERAGE;
    std::vector<double> getReducedResolutionFFT(const std::vector<double>& fftOutputData, int numSections);

private:
    void workerThread();
    void performFFT(const std::vector<double> &data);

    std::thread fftThread;
    std::mutex mtx;
    std::condition_variable cv;
    bool stopThread;
    bool newData;
    std::vector<double> inputData;
    DataAvailableCallback callback;
};

#endif // FFT_PROCESSOR_H
