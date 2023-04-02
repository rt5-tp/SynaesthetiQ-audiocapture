#include <iostream>
#include <stdio.h>
#include <string>
#include <vector>

#include "GenrePredictor.hpp"

using namespace std;


int main(int argc, char** argv) {

    auto pFile = fopen ( "country.wav" , "rb" );
    if (pFile==NULL) {fputs ("File error",stderr); exit (1);}

    // obtain file size:
    fseek (pFile , 0 , SEEK_END);
    auto buffer_size = ftell (pFile);
    rewind (pFile);

    // allocate memory to contain the whole file:
    char* buffer = (char*) malloc (sizeof(char)*buffer_size);
    if (buffer == NULL) {fputs ("Memory error",stderr); exit (2);}

    // copy the file into the buffer:
    auto result = fread (buffer,1,buffer_size,pFile);
    if (result != buffer_size) {fputs ("Reading error",stderr); exit (3);}

    GenrePredictor predictor;
    auto predictions = predictor.predict(buffer, buffer_size);

    for(auto& prediction : predictions)
    {
        cout<<"("<<prediction.first<<","<<prediction.second<<")"<<endl;
    }
}