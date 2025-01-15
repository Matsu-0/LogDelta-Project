CXX = clang++
CXXFLAGS = -std=c++11 -Wall -O2 -I/opt/homebrew/include -I.
LDFLAGS = -L/opt/homebrew/lib -llzma -lz -lzstd

# 检测是否为 ARM 架构（M1/M2 芯片）
UNAME_M := $(shell uname -m)
ifeq ($(UNAME_M),arm64)
    CXXFLAGS += -arch arm64
    LDFLAGS := -arch arm64 $(LDFLAGS)
endif

# 目标文件
TARGET = test1_data_size

# 源文件路径
VPATH = .

# 源文件
SRCS = test1_data_size.cpp \
       record_compress.cpp \
       bit_buffer.cpp \
       bit_packing.cpp \
       distance.cpp \
       qgram_match.cpp \
       utils.cpp \
       rle.cpp

# 生成的对象文件
OBJS = $(SRCS:.cpp=.o)

# 默认目标
all: $(TARGET)

# 链接规则
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET) $(LDFLAGS)
	@echo "Cleaning object files..."
	@rm -f $(OBJS)

# 编译规则
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 清理规则
clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean