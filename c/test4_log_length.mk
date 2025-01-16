CXX = g++
CXXFLAGS = -std=c++11 -Wall -O3
LDFLAGS = -llzma -lz -lzstd

SRCS = test4_log_length.cpp record_compress.cpp utils.cpp rle.cpp distance.cpp bit_buffer.cpp bit_packing.cpp qgram_match.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = test4_log_length

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) 