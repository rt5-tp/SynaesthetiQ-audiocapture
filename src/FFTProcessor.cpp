#include "FFTProcessor.h"

FFTProcessor* FFTProcessor::singleton;

FFTProcessor::FFTProcessor() : stopThread(false), newData(false) {
    singleton = this;
    start();
}

FFTProcessor::~FFTProcessor() {
    stop();
}

void FFTProcessor::start() {
    //Create new thread for FFT processing
    fftThread = std::thread(&FFTProcessor::workerThread, this);
}

void FFTProcessor::stop() {
    if (fftThread.joinable()) {
        {
            std::unique_lock<std::mutex> lock(mtx);
            stopThread = true;
        }
        audio_available.notify_one();
        fftThread.join();
    }
}

void FFTProcessor::audio_callback(const std::vector<short> &data){
    {
        std::unique_lock<std::mutex> lock(singleton->mtx);
        // Normalize the input data
        singleton->inputData.resize(data.size());
        for (size_t i = 0; i < data.size(); ++i) {
            singleton->inputData[i] = static_cast<double>(data[i]) / 32767.0;
        }
        singleton->newData = true;
    }
    singleton->audio_available.notify_one();
}


void FFTProcessor::registerCallback(DataAvailableCallback cb) {
    callback = cb;
}

void FFTProcessor::workerThread() {
    while (true) {

        std::unique_lock<std::mutex> lock(mtx);
        singleton->audio_available.wait(lock, [this]{
            return newData || stopThread;
            });

        if (stopThread) {
            break;
        }

        std::vector<double> dataCopy = inputData;
        newData = false;
        lock.unlock();

        performFFT(dataCopy);
    }
}



void FFTProcessor::performFFT(const std::vector<double>& data) {
    int N = data.size();
    fftw_complex* in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    fftw_complex* out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    fftw_plan p = fftw_plan_dft_1d(N, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

    for (int i = 0; i < N; i++) {
        in[i][0] = data[i];
        in[i][1] = 0;
    }

    fftw_execute(p);
    fftw_destroy_plan(p);
    fftw_free(in);

    std::vector<double> fftOutputData(N / 2);
    double maxAmplitude = 0;
    int maxIndex = 0;
    for (int i = 0; i < N / 2; i++) {
        fftOutputData[i] = sqrt(out[i][0] * out[i][0] + out[i][1] * out[i][1]);
        if (fftOutputData[i] > maxAmplitude) {
            maxAmplitude = fftOutputData[i];
            maxIndex = i;
        }
    }

    fftw_free(out);


    if (callback) {
        callback(fftOutputData);
    }

    double sampleRate = 44100;
    double binWidth = sampleRate / N;
    double frequency = maxIndex * binWidth;
    std::cout << "Most prominent frequency: " << frequency << " Hz" << std::endl;
}





