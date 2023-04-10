#include <vector>
#include <stdint.h>
#include <string>


/**
 * @brief Performs automatic genre classifications based on incoming audio data
 * 
 * Uses the AudioCapture class to get audio, then uses GenrePredictor class to make requests to a 
 * webserver with WAV-formatted audio and receive regular predictions, which are passed to callback function.
 * 
 * !Warning: Singleton class
 * !Do not attempt to instantiate multiple instances of this class
 */
class GenreClassifier {
    static std::vector<int16_t> rec_audio;
    static int samples_length;

    public:
    typedef void (*GenreClassificationCallback)(const std::vector<std::pair<std::string, float>>&);

    /**
     * @brief Registers a callback for passing genre predictions to
     *
     */
    void register_genre_callback(GenreClassificationCallback cb);

    static GenreClassificationCallback callback;

    /**
     * @brief Constructor for the class
     * 
     * Takes in the number of seconds of audio to classify genre by
     */
    GenreClassifier(float seconds=2.0);

    /**
     * @brief Callback function for predicting genre of incoming audio
     * 
     * This function is passed to AudioCapture class, which calls it with available audio data. 
     * It accumulates audio until it has the required length and makes a prediction on the data.
     * Lastly, prediction results are passed onto provided callback function 'callback'
     */
    static void audio_callback(const std::vector<short>& data);        
};