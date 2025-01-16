CXX = g++
CXXFLAGS = -std=c++17 -Wall -O3
LDFLAGS = -llzma -lz -lzstd

SRCS = test3_distance_function.cpp record_compress.cpp utils.cpp rle.cpp distance.cpp bit_buffer.cpp bit_packing.cpp qgram_match.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = test3_distance_function

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) 