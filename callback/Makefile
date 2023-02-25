CC=g++
CFLAGS=-c -Wall
LDFLAGS=-lasound -lSDL2 -lstdc++
SOURCES=AudioCapture.cpp main.cpp 
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=capture

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)
