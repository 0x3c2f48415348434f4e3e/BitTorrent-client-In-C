CC = g++
CFLAGS = -Wall -pedantic
TARGET = bitTorrent
OBJ = $(wildcard *.cpp)
SRC = $(wildcard *.cpp)

$(TARGET) : $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@
%.o : %.cpp
	$(CC) $(CFLAGS) -c %.cpp

clean:
	rm $(TARGET)
