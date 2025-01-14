CXX = clang++
CXXFLAGS = -std=c++11 -Wall -O2 -I/opt/homebrew/include
LDFLAGS = -L/opt/homebrew/lib -llzma -lz -lzstd

# 检测是否为 ARM 架构（M1/M2 芯片）
UNAME_M := $(shell uname -m)
ifeq ($(UNAME_M),arm64)
    CXXFLAGS += -arch arm64
    LDFLAGS := -arch arm64 $(LDFLAGS)
endif

# Source files
SRCS = record_compress.cpp \
       bit_buffer.cpp \
       bit_packing.cpp \
       distance.cpp \
       qgram_match.cpp \
       utils.cpp \
       rle.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Target executable
TARGET = record_compress_test

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
