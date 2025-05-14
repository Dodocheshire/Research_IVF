# 顶层 Makefile
CXX := g++
CXXFLAGS := -O3 -mavx2 -g -fopenmp -std=c++17
INCLUDES := \
  -I/usr/include/eigen3 \
  -I/usr/local/include/eigen3 \
  -I/usr/include/hdf5/serial \
  -I$(abspath src/include) \
  -I$(abspath src/include/utils) \
  -I$(abspath src/include/dim_reduction) \
  -I$(abspath src/include/clustering)
LIBS := -L/usr/lib/x86_64-linux-gnu/hdf5/serial -lhdf5_cpp -lhdf5 -fopenmp

export CXX CXXFLAGS INCLUDES LIBS

BUILD_DIR := build
BIN := $(BUILD_DIR)/main
ASM := $(BIN).asm
MAP := $(BIN).map

.PHONY: all clean

all:
	mkdir -p build
	$(MAKE) -C src 
	$(CXX) $(shell find $(BUILD_DIR) -name '*.o') -o $(BIN) $(LIBS) -Wl,-Map=$(MAP)
	objdump -d -S $(BIN) > $(ASM)

clean:  
	$(MAKE) -C src clean
	rm -rf $(BUILD_DIR)/*