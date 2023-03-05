CC=clang++
CFLAGS=-c -Wall -fsanitize=address -fno-omit-frame-pointer
LDFLAGS=-lasound -lSDL2 -lstdc++ -lpthread
SOURCES=AudioCapture.cpp main.cpp 
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=soundweave

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -fsanitize=address -o $@ $(LDFLAGS)

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE) *.wav *.raw
