CXX=g++
OBJ=bin_info

.PHONY: all clean

all: $(OBJ)

loader.o: includes/loader.cpp
	$(CXX) -std=c++11 -c includes/loader.cpp

ansi_colors.o: includes/ansi_colors.cpp
	$(CXX) -std=c++11 -c includes/ansi_colors.cpp

linear_disassembler.o: includes/linear_disassembler.cpp
	$(CXX) -std=c++11 -c includes/linear_disassembler.cpp

bin_info: loader.o ansi_colors.o linear_disassembler.o bin_info.cpp
	$(CXX) -std=c++11 -o bin_info bin_info.cpp loader.o ansi_colors.o linear_disassembler.o -lbfd -lcapstone

clean:
	rm -f $(OBJ) *.o
