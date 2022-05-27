CXX=g++
OBJ=bin_info

.PHONY: all clean

all: $(OBJ)

loader.o: includes/loader.cpp
	$(CXX) -std=c++11 -c includes/loader.cpp

bin_info: loader.o bin_info.cpp
	$(CXX) -std=c++11 -o bin_info bin_info.cpp loader.o -lbfd

clean:
	rm -f $(OBJ) *.o
