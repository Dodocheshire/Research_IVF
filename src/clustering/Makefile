# src/utils/Makefile
SRC := k_means.cpp IVF.cpp #AdaIVF.cpp B_k_means.cpp 
OBJ := $(SRC:.cpp=.o)
BUILD_DIR := ../../build/clustering

.PHONY: all clean

all: $(addprefix $(BUILD_DIR)/, $(OBJ))

$(BUILD_DIR)/%.o: %.cpp
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(BUILD_DIR)/*.o
