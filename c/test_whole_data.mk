CXX = g++
CXXFLAGS = -std=c++17 -Wall -O2
LDFLAGS = -llzma -lz -lzstd

# Target file
TARGET = test_whole_data

# Source files
SRCS = test_whole_data.cpp \
       record_compress.cpp \
       bit_buffer.cpp \
       bit_packing.cpp \
       distance.cpp \
       qgram_match.cpp \
       utils.cpp \
       rle.cpp \
       variable_length_substitution.cpp \
       ts_2diff.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Default target
all: $(TARGET)

# Main target
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET) $(LDFLAGS)
	@echo "Cleaning object files..."
	@rm -f $(OBJS)

# Compile source files to object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean all generated files
clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean