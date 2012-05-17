CXX=g++
CXXFLAGS=-std=c++11 -Wall -Wextra -pedantic
FILES=addressbus.cpp cartridge.cpp video.cpp z80.cpp

emu:$(FILES:.cpp=.o)
	$(CXX) -o $@ $^ main.cpp $(CXXFLAGS)

