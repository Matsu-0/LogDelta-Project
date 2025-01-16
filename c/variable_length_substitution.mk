CXX = clang++
CXXFLAGS = -std=c++11 -Wall -O2 -I/opt/homebrew/include -I.
LDFLAGS = -L/opt/homebrew/lib -llzma -lz -lzstd

# Check if running on ARM architecture (M1/M2 chip)
UNAME_M := $(shell uname -m)
ifeq ($(UNAME_M),arm64)
    CXXFLAGS += -arch arm64
    LDFLAGS := -arch arm64 $(LDFLAGS)
endif

# Target executable
TARGET = test_variable_length_substitution

# Source file path
VPATH = .

# Source files
SRCS = variable_length_substitution.cpp \
       qgram_match.cpp \
       bit_buffer.cpp \
       bit_packing.cpp \
       distance.cpp \
       utils.cpp \
       rle.cpp

# Generated object files
OBJS = $(SRCS:.cpp=.o)

# Default target
all: $(TARGET)

# Linking rule
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET) $(LDFLAGS)
	@echo "Cleaning object files..."
	@rm -f $(OBJS)

# Compilation rule
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean rule
clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean 