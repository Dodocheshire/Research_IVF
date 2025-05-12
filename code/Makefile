# 编译器
CXX = g++

# 编译选项
CXXFLAGS = -O3 -mavx2 -fopenmp -std=c++17

# 包含路径
INCLUDES = -I /usr/include/eigen3 -I /usr/include/hdf5/serial

# 库路径和链接库
LIBS = -L /usr/lib/x86_64-linux-gnu/hdf5/serial -lhdf5_cpp -lhdf5

# 目标可执行文件
TARGET = ex_rawdim

# 源文件
SRCS = IVF.cpp k_means.cpp main_rawdim.cpp utils_hdf5.cpp

# 生成目标
all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ $(LIBS) -o $@

# 清理
clean:
	rm -f $(TARGET)

.PHONY: all clean