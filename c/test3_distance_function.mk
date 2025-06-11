CXX = g++
CXXFLAGS = -std=c++11 -Wall -O2
LDFLAGS = -llzma -lz -lzstd

# Target file
TARGET = test3_distance_function

# Source files
SRCS = test3_distance_function.cpp \
       record_compress.cpp \
       bit_buffer.cpp \
       bit_packing.cpp \
       distance.cpp \
       qgram_match.cpp \
       variable_length_substitution.cpp \
       utils.cpp \
       rle.cpp \
       ts_2diff.cpp

# Object and dependency files
OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.d)

# Default target
all: $(TARGET)

# Include dependency files
-include $(DEPS)

# Main target
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET) $(LDFLAGS)
	@echo "Cleaning intermediate files..."
	@rm -f $(OBJS) $(DEPS)

# Generate object files and dependency files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

# Clean all generated files
clean:
	rm -f $(OBJS) $(DEPS) $(TARGET)

.PHONY: all clean