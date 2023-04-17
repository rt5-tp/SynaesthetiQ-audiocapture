#ifndef FFT_PROCESSOR_H
#define FFT_PROCESSOR_H

#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <fftw3.h>
#include <cmath>
#include <algorithm>

/**
 * @enum ReductionMethod
 * @brief An enumeration of available methods for reducing the resolution of FFT data.
 */
enum ReductionMethod
{
    AVERAGE,
    MAXIMUM
};

/**
 * @class FFTProcessor
 * @brief A class for processing Fast Fourier Transform (FFT) on audio data.
 */
class FFTProcessor
{
public:
    using DataAvailableCallback = void (*)(const std::vector<double> &);
    using LEDCallback = void (*)(const std::vector<std::vector<bool>> &);

    /**
     * @brief Default constructor for the FFTProcessor class.
     */
    FFTProcessor();

    /**
     * @brief Destructor for the FFTProcessor class.
     */
    ~FFTProcessor();

    /**
     * @brief Starts the worker thread for FFT processing.
     */
    void start();

    /**
     * @brief Stops the worker thread for FFT processing.
     */
    void stop();

    /**
     * @brief Callback function to receive audio data.
     * @param data A vector containing the audio data.
     */
    static void audio_callback(const std::vector<short> &data);

    /**
     * @brief Registers a callback function for FFT data availability.
     * @param cb A function pointer to the callback to be registered.
     */
    void registerCallback(DataAvailableCallback cb);

    /**
     * @brief Registers a callback function for LED matrix data availability.
     * @param cb A function pointer to the callback to be registered.
     */
    void registerLEDCallback(LEDCallback cb);

    ReductionMethod reductionMethod = AVERAGE;

    /**
     * @brief Returns a reduced resolution FFT data vector based on the specified method.
     * @param fftOutputData A vector containing the original FFT output data.
     * @param numSections The number of sections to divide the data into.
     * @return A vector of reduced resolution FFT data.
     */
    std::vector<double> getReducedResolutionFFT(const std::vector<double> &fftOutputData, int numSections);

private:
    /**
     * @brief The worker thread function for FFT processing.
     */
    void workerThread();

    /**
     * @brief Performs FFT on the input data and calls the registered callback functions.
     * @param data A vector containing the input data.
     */
    void performFFT(const std::vector<double> &data);

    /**
     * @brief Converts FFT data into an LED matrix representation.
     * @param fftData A vector containing the FFT data.
     * @param rows The number of rows in the LED matrix.
     * @param cols The number of columns in the LED matrix.
     * @param minFrequency The minimum frequency to include in the matrix.
     * @param maxFrequency The maximum frequency to include in the matrix.
     * @return A 2D vector representing the LED matrix.
     */
    std::vector<std::vector<bool>> convertFFTToLEDMatrix(const std::vector<double> &fftData, int rows, int cols, double minFrequency, double maxFrequency);

    static FFTProcessor *singleton;

    std::thread fftThread;
    std::mutex mtx;
    std::condition_variable audio_available;
    static std::condition_variable saudio_available;
    bool stopThread;
    bool newData;
    std::vector<double> inputData;
    DataAvailableCallback fftCallback;
    LEDCallback ledMatrixCallback;
};

#endif // FFT_PROCESSOR_H