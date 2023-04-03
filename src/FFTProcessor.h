#ifndef FFT_PROCESSOR_H
#define FFT_PROCESSOR_H

#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <fftw3.h>
#include <cmath>

class FFTProcessor {
public:
    using DataAvailableCallback = void(*)(const std::vector<double> &);

    FFTProcessor();
    ~FFTProcessor();

    void start();
    void stop();
    void processData(const std::vector<short> &data);
    void registerCallback(DataAvailableCallback cb);

private:
    void workerThread();
    void performFFT(const std::vector<short> &data);

    std::thread fftThread;
    std::mutex mtx;
    std::condition_variable cv;
    bool stopThread;
    bool newData;
    std::vector<short> inputData;
    DataAvailableCallback callback;
};

#endif // FFT_PROCESSOR_H
