CXX = g++
CXXFLAGS = -std=c++17 -Wall -O2 -I../lib/include
LDFLAGS = -llzma -lz -lzstd -lbz2 ../lib/lib/liblz4.a

# Common source files
COMMON_SRCS = bit_buffer.cpp \
       bit_packing.cpp \
       distance.cpp \
       qgram_match.cpp \
       utils.cpp \
       rle.cpp \
       variable_length_substitution.cpp \
       ts_2diff.cpp

# Main program source files
SRCS = record_compress.cpp $(COMMON_SRCS)
DECOMPRESS_SRCS = record_decompress.cpp $(COMMON_SRCS)


# Object and dependency files
OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.d)
DECOMPRESS_OBJS = $(DECOMPRESS_SRCS:.cpp=.o)
DECOMPRESS_DEPS = $(DECOMPRESS_SRCS:.cpp=.d)


# Target executables
COMPRESS_TARGET = record_compress
DECOMPRESS_TARGET = record_decompress
RLE_TEST_TARGET = rle_test


# Default target
all: $(COMPRESS_TARGET) $(DECOMPRESS_TARGET)
	@echo "Cleaning intermediate files..."
	@rm -f $(OBJS) $(DEPS) $(DECOMPRESS_OBJS) $(DECOMPRESS_DEPS)

# Include dependency files
-include $(DEPS) $(DECOMPRESS_DEPS)

# Main compression target
$(COMPRESS_TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -DRECORD_COMPRESS $(OBJS) -o $(COMPRESS_TARGET) $(LDFLAGS)

# Main decompression target
$(DECOMPRESS_TARGET): $(DECOMPRESS_OBJS)
	$(CXX) $(CXXFLAGS) -DRECORD_DECOMPRESS $(DECOMPRESS_OBJS) -o $(DECOMPRESS_TARGET) $(LDFLAGS)
	@echo "Cleaning intermediate files..."
	@rm -f $(DECOMPRESS_OBJS) $(DECOMPRESS_DEPS)

# RLE test target
$(RLE_TEST_TARGET): rle.o utils.o
	$(CXX) $(CXXFLAGS) -DRLE_TEST rle.o utils.o -o $(RLE_TEST_TARGET)


# Generate object files and dependency files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

# Add macro for record_compress.o
record_compress.o: record_compress.cpp
	$(CXX) $(CXXFLAGS) -DRECORD_COMPRESS -MMD -MP -c $< -o $@

# Add macro for record_decompress.o
record_decompress.o: record_decompress.cpp
	$(CXX) $(CXXFLAGS) -DRECORD_DECOMPRESS -MMD -MP -c $< -o $@



# Simple targets
decomp: $(DECOMPRESS_TARGET)
	@echo "Cleaning intermediate files..."
	@rm -f $(DECOMPRESS_OBJS) $(DECOMPRESS_DEPS)

comp: $(COMPRESS_TARGET)
	@echo "Cleaning intermediate files..."
	@rm -f $(OBJS) $(DEPS)


# Clean all generated files
clean:
	rm -f $(OBJS) $(DEPS) $(DECOMPRESS_OBJS) $(DECOMPRESS_DEPS) \
	       $(COMPRESS_TARGET) $(DECOMPRESS_TARGET) $(RLE_TEST_TARGET)

.PHONY: all clean comp decomp
