# src/utils/Makefile
SRC := utils.cpp utils_hdf5.cpp
OBJ := $(SRC:.cpp=.o)
BUILD_DIR := ../../build/utils

.PHONY: all clean

all: $(addprefix $(BUILD_DIR)/, $(OBJ))

$(BUILD_DIR)/%.o: %.cpp
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(BUILD_DIR)/*.o
