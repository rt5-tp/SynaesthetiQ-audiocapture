#include <iostream>
#include <SDL2/SDL.h>
#include <alsa/asoundlib.h>
#include <fstream>
#include <vector>
#include <climits>
#include <thread>
#include <string>
#include <signal.h>
#include <fftw3.h>
#include <cstring>
#include <cmath>
#include <algorithm>

#include "AudioCapture.h"
#include "FFTProcessor.h"
#include "GenreClassifier.h"

FFTProcessor fftProcessor; // create instance of fftprocessor class

void data_available_callback(const std::vector<short> &data)
{

    fftProcessor.processData(data);

    // Callback function for the AudioCapture class, currenlty unused
    // std::cout << "Data available: " << data.size() << " samples" << std::endl;
}

void genre_prediction_callback(const std::vector<std::pair<std::string, float>> &predictions)
{
    std::cout << "\nGenre Predictions callback: \n";
    for (auto prediction : predictions)
    {
        std::cout << prediction.first << " : " << prediction.second << std::endl;
    }
    std::cout << std::endl;
}

std::vector<std::vector<bool>> convertFFTToLEDMatrix(const std::vector<double> &fftData, int rows, int cols, double maxFrequency)
{
    std::vector<std::vector<bool>> ledMatrix(rows, std::vector<bool>(cols, false));

    double maxAmplitude = *std::max_element(fftData.begin(), fftData.end());

    double minFrequencyLog = std::log10(1);
    double maxFrequencyLog = std::log10(maxFrequency);
    double logRange = maxFrequencyLog - minFrequencyLog;

    for (int i = 0; i < rows; i++)
    {
        // std::cout << "i test = " << i << std::endl;
        for (int j = 0; j < cols; j++)
        {
            double targetFrequencyLog = minFrequencyLog + (j * logRange) / cols;
            double targetFrequency = std::pow(10, targetFrequencyLog);
            double lowerBound = targetFrequency - targetFrequency / 2;
            double upperBound = targetFrequency + targetFrequency / 2;

            // std::cout << "j test = " << j << std::endl;

            double maxValueInBin = 0;
            int lowerIndex = static_cast<int>(lowerBound / maxFrequency * fftData.size());
            int upperIndex = std::min(static_cast<int>(upperBound / maxFrequency * fftData.size()), static_cast<int>(fftData.size()));


            for (int k = lowerIndex; k < upperIndex; ++k)
            {
                maxValueInBin = std::max(maxValueInBin, fftData[k]);
            }

            double minValueThreshold = 0.1; // You can adjust this value to your preference
            double normalizedAmplitude = maxValueInBin / maxAmplitude;

            if (normalizedAmplitude > minValueThreshold)
            {
                int threshold = static_cast<int>(normalizedAmplitude * rows);
                if (i < threshold)
                {
                    ledMatrix[rows - i - 1][j] = true;
                }
            }
        }
    }

    return ledMatrix;
}

void printLEDMatrix(const std::vector<std::vector<bool>> &ledMatrix)
{
    int rows = ledMatrix.size();
    int cols = ledMatrix[0].size();

    for (int i = 0; i < rows; ++i)
    {
        for (int j = 0; j < cols; ++j)
        {
            std::cout << (ledMatrix[i][j] ? "#" : " ");
        }
        std::cout << std::endl;
    }
}

// FFT callback function
void onFFTDataAvailable(const std::vector<double> &data)
{
    std::cout << "FFT DATA AVAILABLE" << std::endl;
    int rows = 16;
    int cols = 32;
    double maxFrequency = 16000;
    std::vector<std::vector<bool>> ledMatrix = convertFFTToLEDMatrix(data, rows, cols, maxFrequency);
    printLEDMatrix(ledMatrix);

    // Output the LED matrix to your physical display
}

int main(int argc, char *argv[])
{
    std::cout << "Initialising!" << std::endl;
    bool sdl_enabled = false;
    std::string device_name = "";

    for (int i = 0; i < argc; i++)
    {
        std::string arg = argv[i];
        int iarg = atoi(arg.c_str());
        if (arg == "sdl")
        {
            sdl_enabled = true;
        }
        else if (iarg != 0)
        {
            device_name = arg;
        }
    }

    GenreClassifier classifier;

    try
    {

        classifier.register_genre_callback(&genre_prediction_callback);

        AudioCapture audioCapture(device_name, sdl_enabled);
        audioCapture.register_callback(classifier.audio_callback);
        audioCapture.register_callback(&data_available_callback);

        // update callbacks for consistency
        fftProcessor.registerCallback(onFFTDataAvailable);

        std::cout << "Starting" << std::endl;
        std::cout << "Waiting...\n";
        std::this_thread::sleep_for(std::chrono::seconds(5));
        std::cout << "Done.\n";
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }

    std::cout << "Complete" << std::endl;
    return 0;
}
