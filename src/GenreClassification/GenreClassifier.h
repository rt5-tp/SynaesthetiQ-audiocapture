#include <vector>
#include <stdint.h>
#include <string>

// Warning: Singleton class
// Do not attempt to instantiate multiple instances of this class
class GenreClassifier {
    static std::vector<int16_t> rec_audio;
    static int samples_length;

    public:
    typedef void (*GenreClassificationCallback)(const std::vector<std::pair<std::string, float>>&);

    void register_genre_callback(GenreClassificationCallback cb);

    static GenreClassificationCallback callback;

    GenreClassifier(float seconds=2.0);

    static void audio_callback(const std::vector<short>& data);        
};