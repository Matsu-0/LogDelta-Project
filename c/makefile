CXX = g++
CXXFLAGS = -std=c++17 -Wall -O2 -I../lib/include
LDFLAGS = -llzma -lz -lzstd ../lib/lib/liblz4.a

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

# Test program source files
TEST1_SRCS = test1_data_size.cpp record_compress.cpp bit_buffer.cpp bit_packing.cpp distance.cpp qgram_match.cpp utils.cpp rle.cpp variable_length_substitution.cpp ts_2diff.cpp
TEST2_SRCS = test2_compressor.cpp record_compress.cpp bit_buffer.cpp bit_packing.cpp distance.cpp qgram_match.cpp utils.cpp rle.cpp variable_length_substitution.cpp ts_2diff.cpp
TEST3_SRCS = test3_distance_function.cpp record_compress.cpp bit_buffer.cpp bit_packing.cpp distance.cpp qgram_match.cpp utils.cpp rle.cpp variable_length_substitution.cpp ts_2diff.cpp
TEST4_SRCS = test4_block_size.cpp record_compress.cpp bit_buffer.cpp bit_packing.cpp distance.cpp qgram_match.cpp utils.cpp rle.cpp variable_length_substitution.cpp ts_2diff.cpp
TEST5_SRCS = test5_window_size.cpp record_compress.cpp bit_buffer.cpp bit_packing.cpp distance.cpp qgram_match.cpp utils.cpp rle.cpp variable_length_substitution.cpp ts_2diff.cpp
TEST_WHOLE_SRCS = test_whole_data.cpp record_compress.cpp bit_buffer.cpp bit_packing.cpp distance.cpp qgram_match.cpp utils.cpp rle.cpp variable_length_substitution.cpp ts_2diff.cpp
TEST6_SRCS = test6_disturbing_degree.cpp record_compress.cpp bit_buffer.cpp bit_packing.cpp distance.cpp qgram_match.cpp utils.cpp rle.cpp variable_length_substitution.cpp ts_2diff.cpp

# Object and dependency files
OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.d)
DECOMPRESS_OBJS = $(DECOMPRESS_SRCS:.cpp=.o)
DECOMPRESS_DEPS = $(DECOMPRESS_SRCS:.cpp=.d)

# Test program object files
TEST1_OBJS = test1_data_size.o record_compress_test.o bit_buffer.o bit_packing.o distance.o qgram_match.o utils.o rle.o variable_length_substitution.o ts_2diff.o
TEST1_DEPS = test1_data_size.d record_compress_test.d bit_buffer.d bit_packing.d distance.d qgram_match.d utils.d rle.d variable_length_substitution.d ts_2diff.d
TEST2_OBJS = test2_compressor.o record_compress_test.o bit_buffer.o bit_packing.o distance.o qgram_match.o utils.o rle.o variable_length_substitution.o ts_2diff.o
TEST2_DEPS = test2_compressor.d record_compress_test.d bit_buffer.d bit_packing.d distance.d qgram_match.d utils.d rle.d variable_length_substitution.d ts_2diff.d
TEST3_OBJS = test3_distance_function.o record_compress_test.o bit_buffer.o bit_packing.o distance.o qgram_match.o utils.o rle.o variable_length_substitution.o ts_2diff.o
TEST3_DEPS = test3_distance_function.d record_compress_test.d bit_buffer.d bit_packing.d distance.d qgram_match.d utils.d rle.d variable_length_substitution.d ts_2diff.d
TEST4_OBJS = test4_block_size.o record_compress_test.o bit_buffer.o bit_packing.o distance.o qgram_match.o utils.o rle.o variable_length_substitution.o ts_2diff.o
TEST4_DEPS = test4_block_size.d record_compress_test.d bit_buffer.d bit_packing.d distance.d qgram_match.d utils.d rle.d variable_length_substitution.d ts_2diff.d
TEST5_OBJS = test5_window_size.o record_compress_test.o bit_buffer.o bit_packing.o distance.o qgram_match.o utils.o rle.o variable_length_substitution.o ts_2diff.o
TEST5_DEPS = test5_window_size.d record_compress_test.d bit_buffer.d bit_packing.d distance.d qgram_match.d utils.d rle.d variable_length_substitution.d ts_2diff.d
TEST_WHOLE_OBJS = test_whole_data.o record_compress_test.o bit_buffer.o bit_packing.o distance.o qgram_match.o utils.o rle.o variable_length_substitution.o ts_2diff.o
TEST_WHOLE_DEPS = test_whole_data.d record_compress_test.d bit_buffer.d bit_packing.d distance.d qgram_match.d utils.d rle.d variable_length_substitution.d ts_2diff.d
TEST6_OBJS = test6_disturbing_degree.o record_compress_test.o bit_buffer.o bit_packing.o distance.o qgram_match.o utils.o rle.o variable_length_substitution.o ts_2diff.o
TEST6_DEPS = test6_disturbing_degree.d record_compress_test.d bit_buffer.d bit_packing.d distance.d qgram_match.d utils.d rle.d variable_length_substitution.d ts_2diff.d

# Target executables
COMPRESS_TARGET = record_compress
DECOMPRESS_TARGET = record_decompress
RLE_TEST_TARGET = rle_test

# Test program targets
TEST1_TARGET = test1_data_size
TEST2_TARGET = test2_compressor
TEST3_TARGET = test3_distance_function
TEST4_TARGET = test4_block_size
TEST5_TARGET = test5_window_size
TEST_WHOLE_TARGET = test_whole_data
TEST6_TARGET = test6_disturbing_degree

# Default target
all: $(COMPRESS_TARGET) $(DECOMPRESS_TARGET)
	@echo "Cleaning intermediate files..."
	@rm -f $(OBJS) $(DEPS) $(DECOMPRESS_OBJS) $(DECOMPRESS_DEPS)

# Include dependency files
-include $(DEPS) $(DECOMPRESS_DEPS) $(TEST1_DEPS) $(TEST2_DEPS) $(TEST3_DEPS) $(TEST4_DEPS) $(TEST5_DEPS) $(TEST_WHOLE_DEPS) $(TEST6_DEPS)

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

# Test program targets
$(TEST1_TARGET): $(TEST1_OBJS)
	$(CXX) $(CXXFLAGS) -DTEST_MODE $(TEST1_OBJS) -o $(TEST1_TARGET) $(LDFLAGS)
	@echo "Cleaning intermediate files..."
	@rm -f $(TEST1_OBJS) $(TEST1_DEPS)

$(TEST2_TARGET): $(TEST2_OBJS)
	$(CXX) $(CXXFLAGS) -DTEST_MODE $(TEST2_OBJS) -o $(TEST2_TARGET) $(LDFLAGS)
	@echo "Cleaning intermediate files..."
	@rm -f $(TEST2_OBJS) $(TEST2_DEPS)

$(TEST3_TARGET): $(TEST3_OBJS)
	$(CXX) $(CXXFLAGS) -DTEST_MODE $(TEST3_OBJS) -o $(TEST3_TARGET) $(LDFLAGS)
	@echo "Cleaning intermediate files..."
	@rm -f $(TEST3_OBJS) $(TEST3_DEPS)

$(TEST4_TARGET): $(TEST4_OBJS)
	$(CXX) $(CXXFLAGS) -DTEST_MODE $(TEST4_OBJS) -o $(TEST4_TARGET) $(LDFLAGS)
	@echo "Cleaning intermediate files..."
	@rm -f $(TEST4_OBJS) $(TEST4_DEPS)

$(TEST5_TARGET): $(TEST5_OBJS)
	$(CXX) $(CXXFLAGS) -DTEST_MODE $(TEST5_OBJS) -o $(TEST5_TARGET) $(LDFLAGS)
	@echo "Cleaning intermediate files..."
	@rm -f $(TEST5_OBJS) $(TEST5_DEPS)

$(TEST_WHOLE_TARGET): $(TEST_WHOLE_OBJS)
	$(CXX) $(CXXFLAGS) -DTEST_MODE $(TEST_WHOLE_OBJS) -o $(TEST_WHOLE_TARGET) $(LDFLAGS)
	@echo "Cleaning intermediate files..."
	@rm -f $(TEST_WHOLE_OBJS) $(TEST_WHOLE_DEPS)

$(TEST6_TARGET): $(TEST6_OBJS)
	$(CXX) $(CXXFLAGS) -DTEST_MODE $(TEST6_OBJS) -o $(TEST6_TARGET) $(LDFLAGS)
	@echo "Cleaning intermediate files..."
	@rm -f $(TEST6_OBJS) $(TEST6_DEPS)

# Generate object files and dependency files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

# Add macro for record_compress.o
record_compress.o: record_compress.cpp
	$(CXX) $(CXXFLAGS) -DRECORD_COMPRESS -MMD -MP -c $< -o $@

# Add macro for record_decompress.o
record_decompress.o: record_decompress.cpp
	$(CXX) $(CXXFLAGS) -DRECORD_DECOMPRESS -MMD -MP -c $< -o $@

# Compile record_compress.o for test program (excluding main function)
record_compress_test.o: record_compress.cpp
	$(CXX) $(CXXFLAGS) -DTEST_MODE -MMD -MP -c $< -o $@

# Test targets
test1: $(TEST1_TARGET)
test2: $(TEST2_TARGET)
test3: $(TEST3_TARGET)
test4: $(TEST4_TARGET)
test5: $(TEST5_TARGET)
test-whole: $(TEST_WHOLE_TARGET)
test6: $(TEST6_TARGET)

# Simple targets
decomp: $(DECOMPRESS_TARGET)
	@echo "Cleaning intermediate files..."
	@rm -f $(DECOMPRESS_OBJS) $(DECOMPRESS_DEPS)

comp: $(COMPRESS_TARGET)
	@echo "Cleaning intermediate files..."
	@rm -f $(OBJS) $(DEPS)

# All tests
tests: test1 test2 test3 test4 test5 test-whole test6

# Clean all generated files
clean:
	rm -f $(OBJS) $(DEPS) $(DECOMPRESS_OBJS) $(DECOMPRESS_DEPS) \
	       $(TEST1_OBJS) $(TEST1_DEPS) $(TEST2_OBJS) $(TEST2_DEPS) \
	       $(TEST3_OBJS) $(TEST3_DEPS) $(TEST4_OBJS) $(TEST4_DEPS) \
	       $(TEST5_OBJS) $(TEST5_DEPS) $(TEST_WHOLE_OBJS) $(TEST_WHOLE_DEPS) \
	       $(TEST6_OBJS) $(TEST6_DEPS) \
	       $(COMPRESS_TARGET) $(DECOMPRESS_TARGET) $(RLE_TEST_TARGET) \
	       $(TEST1_TARGET) $(TEST2_TARGET) $(TEST3_TARGET) $(TEST4_TARGET) $(TEST5_TARGET) $(TEST_WHOLE_TARGET) $(TEST6_TARGET)

.PHONY: all clean test1 test2 test3 test4 test5 test-whole test6 tests comp decomp
