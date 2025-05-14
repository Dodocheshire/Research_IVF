# src/Makefile

.PHONY: all clean

all:
	$(MAKE) -C utils
	$(MAKE) -C clustering
	$(MAKE) -C dim_reduction
	$(MAKE) main

main:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c main_rawdim.cpp -o ../build/main.o

clean:
	$(MAKE) -C utils clean
	$(MAKE) -C clustering clean
	$(MAKE) -C dim_reduction clean
	rm -f ../build/*.o
