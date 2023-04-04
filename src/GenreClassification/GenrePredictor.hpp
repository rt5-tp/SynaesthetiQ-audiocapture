
#include <curl/curl.h>
#include "json.hpp"
using namespace std;
using json = nlohmann::json;

class GenrePredictor{

    private:
    static size_t writeFunction(void* ptr, size_t size, size_t nmemb, std::string* data) {
        data->append((char*)ptr, size * nmemb);
        return size * nmemb;
    }

    public:

    GenrePredictor(){
        curl_global_init(CURL_GLOBAL_DEFAULT);
        
    }

    vector<pair<string, float>> predict(char*buffer, int buffer_size){
        vector<pair<string, float>> predictions;

        CURL* curl = curl_easy_init();
        if (!curl) {
            throw exception();
        }
        curl_easy_setopt(curl, CURLOPT_URL, "http://127.0.0.1:5000/predict");
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
        curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
        curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);

        curl_mime *mime;
        curl_mimepart *part;
        /* Create the form */
        mime = curl_mime_init(curl);
    
        /* Fill in the file upload field */
        part = curl_mime_addpart(mime);
        curl_mime_data(part, buffer, buffer_size);
        curl_mime_name(part, "file");
        curl_mime_filename(part, "audio.wav");

        curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);

        std::string response_string;
        
        std::string header_string;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunction);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &header_string);

        curl_easy_perform(curl);
        curl_mime_free(mime);
        curl_easy_cleanup(curl);

        if(response_string.length() <3){
            return predictions;
        }

        auto response_json = json::parse(response_string);

        for (auto& element : response_json) {
            predictions.push_back(make_pair(element["genre"], element["certainty"]));
        }

        return predictions;
    }

    ~GenrePredictor(){
        curl_global_cleanup();
    }
};