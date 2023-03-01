CC=g++
CFLAGS=-c -Wall -g
LDFLAGS=-lasound -lSDL2 -lstdc++ -lpthread
SOURCES=AudioCapture.cpp main.cpp 
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=soundweave

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE) *.wav *.raw
