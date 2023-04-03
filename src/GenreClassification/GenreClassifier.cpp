#include "GenreClassifier.h"
#include "cpp_genre_classifier/GenrePredictor.hpp"
#include <cstring>
#include <string>
#include <stdio.h>
#include <fstream>

std::vector<int16_t> GenreClassifier::rec_audio;
int GenreClassifier::samples_length = 44100*2.0;
GenreClassifier::GenreClassificationCallback GenreClassifier::callback;

GenrePredictor predictor;


class RawAudioWriter {
public:
    RawAudioWriter(const std::string& filename) : filename_(filename) {
        file_.open(filename_, std::ios::binary);
        if (!file_.is_open()) {
            throw std::runtime_error("Could not open file for writing.");
        }
    }

void WriteData(const std::vector<short>& buffer) {
    file_.write(reinterpret_cast<const char*>(buffer.data()), buffer.size() * sizeof(short));
}

void WriteData(const std::vector<char>& buffer) {
    file_.write(buffer.data(), buffer.size());
}

    ~RawAudioWriter() {
        if (file_.is_open()) {
            file_.close();
        }
    }

private:
    std::string filename_;
    std::ofstream file_;
};

std::vector<char> convertRawAudioToWav(std::vector<int16_t> rawAudioData)
{
    std::vector<char> outBuffer;
    // Write WAV header to output buffer
    const int16_t numChannels = 1;
    const int sampleRate = 44100;
    const int numSamples = rawAudioData.size();
    const int32_t subchunk1Size = 16;
    const int16_t audioFormat = 1;
    const int32_t byteRate = sampleRate * sizeof(int16_t) * numChannels;
    const int16_t blockAlign = sizeof(int16_t) * numChannels;
    const int16_t blob = sizeof(int16_t) * 8;
    const int32_t subchunk2Size = numSamples * sizeof(int16_t) * numChannels;
    const int32_t chunkSize = subchunk2Size + 44;

    outBuffer.resize(chunkSize);

    std::memcpy(&outBuffer[0], "RIFF", 4);
    std::memcpy(&outBuffer[4], &chunkSize, 4);
    std::memcpy(&outBuffer[8], "WAVE", 4);
    std::memcpy(&outBuffer[12], "fmt ", 4);
    std::memcpy(&outBuffer[16], &subchunk1Size, 4);
    std::memcpy(&outBuffer[20], &audioFormat, 2);
    std::memcpy(&outBuffer[22], &numChannels, 2);
    std::memcpy(&outBuffer[24], &sampleRate, 4);
    std::memcpy(&outBuffer[28], &byteRate, 4);
    std::memcpy(&outBuffer[32], &blockAlign, 2);
    std::memcpy(&outBuffer[34], &blob, 2);
    std::memcpy(&outBuffer[36], "data", 4);
    std::memcpy(&outBuffer[40], &subchunk2Size, 4);

    
    // Write raw audio data to output buffer
    std::memcpy(&outBuffer[44], rawAudioData.data(), numSamples * sizeof(int16_t) * numChannels);
    return outBuffer;
}

RawAudioWriter wavWriter("wavy.wav");

GenreClassifier::GenreClassifier(float seconds){
    samples_length = 44100*seconds;
}

static bool done = false;

void GenreClassifier::audio_callback(const std::vector<short>& data){
    if(rec_audio.size()<samples_length){
        rec_audio.insert(rec_audio.end(), data.begin(), data.end());
    }
    else {
        if(done) return;
        //rec_audio.clear();
        done = true;

        std::vector<char> wavData = convertRawAudioToWav(rec_audio);
        auto predictions = predictor.predict(wavData.data(), wavData.size());
        //wavWriter.WriteData(wavData);
        // make request
        callback(predictions);
        // call callback
    }
} 

void GenreClassifier::register_genre_callback(GenreClassificationCallback cb){
    GenreClassifier::callback = cb;
}
