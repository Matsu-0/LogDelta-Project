CXX = g++
CXXFLAGS = -std=c++11 -Wall -O2
LDFLAGS = -llzma -lz -lzstd

# Source files
SRCS = record_compress.cpp \
       bit_buffer.cpp \
       bit_packing.cpp \
       distance.cpp \
       qgram_match.cpp \
       utils.cpp \
       rle.cpp \
       variable_length_substitution.cpp

DECOMPRESS_SRCS = record_decompress.cpp \
       bit_buffer.cpp \
       bit_packing.cpp \
       distance.cpp \
       qgram_match.cpp \
       utils.cpp \
       rle.cpp \
       variable_length_substitution.cpp

# Object and dependency files
OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.d)
DECOMPRESS_OBJS = $(DECOMPRESS_SRCS:.cpp=.o)
DECOMPRESS_DEPS = $(DECOMPRESS_SRCS:.cpp=.d)

# Target executables
COMPRESS_TARGET = record_compress
DECOMPRESS_TARGET = record_decompress

# Default target
all: $(COMPRESS_TARGET) $(DECOMPRESS_TARGET)

# Include dependency files
-include $(DEPS)

# Main compress target
$(COMPRESS_TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -DRECORD_COMPRESS $(OBJS) -o $(COMPRESS_TARGET) $(LDFLAGS)
	@echo "Cleaning intermediate files..."
	@rm -f $(OBJS) $(DEPS)

# Main decompress target
$(DECOMPRESS_TARGET): $(DECOMPRESS_OBJS)
	$(CXX) $(CXXFLAGS) -DRECORD_DECOMPRESS $(DECOMPRESS_OBJS) -o $(DECOMPRESS_TARGET) $(LDFLAGS)
	@echo "Cleaning intermediate files..."
	@rm -f $(DECOMPRESS_OBJS) $(DECOMPRESS_DEPS)

# Generate object files and dependency files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

# 为record_compress.o单独加宏
record_compress.o: record_compress.cpp
	$(CXX) $(CXXFLAGS) -DRECORD_COMPRESS -MMD -MP -c $< -o $@

# 为record_decompress.o单独加宏
record_decompress.o: record_decompress.cpp
	$(CXX) $(CXXFLAGS) -DRECORD_DECOMPRESS -MMD -MP -c $< -o $@

# Clean all generated files
clean:
	rm -f $(OBJS) $(DEPS) $(DECOMPRESS_OBJS) $(DECOMPRESS_DEPS) $(COMPRESS_TARGET) $(DECOMPRESS_TARGET)

.PHONY: all clean
