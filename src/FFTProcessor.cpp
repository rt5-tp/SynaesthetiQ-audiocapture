#include "FFTProcessor.h"

FFTProcessor *FFTProcessor::singleton;

FFTProcessor::FFTProcessor() : stopThread(false), newData(false)
{
    singleton = this;
    start();
}

FFTProcessor::~FFTProcessor()
{
    stop();
}

void FFTProcessor::start()
{
    // Create new thread for FFT processing
    fftThread = std::thread(&FFTProcessor::workerThread, this);
}

void FFTProcessor::stop()
{
    if (fftThread.joinable())
    {
        {
            std::unique_lock<std::mutex> lock(mtx);
            stopThread = true;
        }
        audio_available.notify_one();
        fftThread.join();
    }
}

void FFTProcessor::audio_callback(const std::vector<short> &data)
{
    {
        std::unique_lock<std::mutex> lock(singleton->mtx);
        // Normalize the input data
        singleton->inputData.resize(data.size());
        for (size_t i = 0; i < data.size(); ++i)
        {
            singleton->inputData[i] = static_cast<double>(data[i]) / 32767.0;
        }
        singleton->newData = true;
    }
    singleton->audio_available.notify_one();
}

void FFTProcessor::registerCallback(DataAvailableCallback cb)
{
    fftCallback = cb;
}

void FFTProcessor::registerLEDCallback(LEDCallback cb)
{
    ledMatrixCallback = cb;
}

void FFTProcessor::workerThread()
{
    while (true)
    {

        std::unique_lock<std::mutex> lock(mtx);
        singleton->audio_available.wait(lock, [this]
                                        { return newData || stopThread; });

        if (stopThread)
        {
            break;
        }

        std::vector<double> dataCopy = inputData;
        newData = false;
        lock.unlock();

        performFFT(dataCopy);
    }
}

void FFTProcessor::performFFT(const std::vector<double> &data)
{
    int N = data.size();
    fftw_complex *in = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * N);
    fftw_complex *out = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * N);
    fftw_plan p = fftw_plan_dft_1d(N, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

    for (int i = 0; i < N; i++)
    {
        in[i][0] = data[i];
        in[i][1] = 0;
    }

    fftw_execute(p);
    fftw_destroy_plan(p);
    fftw_free(in);

    std::vector<double> fftOutputData(N / 2);
    double maxAmplitude = 0;
    int maxIndex = 0;
    for (int i = 0; i < N / 2; i++)
    {
        fftOutputData[i] = sqrt(out[i][0] * out[i][0] + out[i][1] * out[i][1]);
        if (fftOutputData[i] > maxAmplitude)
        {
            maxAmplitude = fftOutputData[i];
            maxIndex = i;
        }
    }

    fftw_free(out);

    // Invoke the FFT callback function
    if (fftCallback)
    {
        fftCallback(fftOutputData);
    }

    // std::cout << "sending data" << std::endl;
    if (ledMatrixCallback)
    {
        int rows = 16;
        int cols = 32;
        double minFrequency = 60;                                                                                                // Set minimum frequency for visualisation
        double maxFrequency = 16000;                                                                                             // Set maximum frequency for visualisation
        std::vector<std::vector<bool>> ledMatrix = convertFFTToLEDMatrix(fftOutputData, rows, cols, minFrequency, maxFrequency); // 2D boolean vector to represent LED matrix
        ledMatrixCallback(ledMatrix);
    }

    double sampleRate = 44100;
    double binWidth = sampleRate / N;
    double frequency = maxIndex * binWidth;
    std::cout << "Most prominent frequency: " << frequency << " Hz" << std::endl;
}

/**
 * This function takes in FFT data, number of rows, number of columns, and a max frequency, and outputs a 2D matrix of bools.
 * @param fftData a vector of double values representing the FFT data
 * @param rows an integer representing the number of rows in the output LED matrix
 * @param cols an integer representing the number of columns in the output LED matrix
 * @param minFrequency a double representing the minimum frequency of the LED matrix display
 * @param maxFrequency a double representing the maximum frequency of the LED matrix display
 * @return a 2D vector of bool values representing the LED matrix display
 */
std::vector<std::vector<bool>> FFTProcessor::convertFFTToLEDMatrix(const std::vector<double> &fftData, int rows, int cols, double minFrequency, double maxFrequency)
{
    // Creates a 2D matrix of bools with size rows x cols, initialized to false
    std::vector<std::vector<bool>> ledMatrix(rows, std::vector<bool>(cols, false));

    // Finds the maximum amplitude in the FFT data
    double maxAmplitude = *std::max_element(fftData.begin(), fftData.end());

    // Sets the logarithmic frequency range of the LED matrix display
    double minFrequencyLog = std::log10(minFrequency);   // Set the minimum frequency cutoff to 30 Hz
    double maxFrequencyLog = std::log10(maxFrequency);   // Set the maximum frequency cutoff to the input max frequency
    double logRange = maxFrequencyLog - minFrequencyLog; // Calculate the logarithmic range of the frequency scale

    // Sets a cutoff threshold to ignore values below a certain amount
    double cutoffThreshold = 1; // If the amplitude value is less than this, don't show it on the LED matrix
    // std::cout << "Cutoff = " << cutoffThreshold << std::endl; // Output the cutoff threshold value for debugging

    // Calculates the scaling factor for the amplitudes based on the number of rows in the LED matrix display
    double yScaleFactor = static_cast<double>(rows) / maxAmplitude;

    // Loops through each row and column in the LED matrix display
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            // Calculates the center frequency of the current column
            double targetFrequencyLog = minFrequencyLog + (j * logRange) / cols; // Calculate the logarithmic frequency of the center of the current column
            double targetFrequency = std::pow(10, targetFrequencyLog);           // Convert the logarithmic frequency to linear frequency
            double lowerBound = targetFrequency - targetFrequency / 2;           // Calculate the lower bound of the target frequency range
            double upperBound = targetFrequency + targetFrequency / 2;           // Calculate the upper bound of the target frequency range

            // Searches for the maximum amplitude value in the target frequency range
            double maxValueInBin = 0;
            int lowerIndex = static_cast<int>(lowerBound / maxFrequency * fftData.size());                                             // Find the lower index valuee in the bin
            int upperIndex = std::min(static_cast<int>(upperBound / maxFrequency * fftData.size()), static_cast<int>(fftData.size())); // Find the upper index value in the bin
            for (int k = lowerIndex; k < upperIndex; k++)
            {
                maxValueInBin = std::max(maxValueInBin, fftData[k]);
            }
            // std::cout << "max value in bin = " << maxValueInBin << std::endl; // Output the maximum amplitude value in the current bin for debugging

            // If the maximum amplitude value is above the cutoff threshold, set the corresponding LED matrix element to true
            if (maxValueInBin > cutoffThreshold)
            {
                double scaledAmplitude = maxValueInBin * yScaleFactor; // Scale the amplitude value based on the number of rows in the LED matrix display
                // std::cout << "Scaled Amplitude at bin " << j << ": " << scaledAmplitude << std::endl; // Output the scaled amplitude value for debugging
                int threshold = static_cast<int>(scaledAmplitude);
                if (i < threshold)
                {
                    ledMatrix[rows - i - 1][j] = true; // If the current row index is below the scaled amplitude value, set the corresponding LED matrix element to true
                }
            }
        }
    }

    return ledMatrix; // Return the final LED matrix display
}