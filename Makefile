.PHONY: all clean

CXX=g++
CXXFLAGS=-std=c++17 -Wall -pedantic

all: hw_02

obj:
	mkdir -p obj

hw_02: src/main.cpp obj/huffman.o include/*.h obj
	$(CXX) $(CXXFLAGS) -o $@ -Iinclude $< obj/*

test: test/huffman_test.cpp obj/huffman.o include/*h obj
	$(CXX) $(CXXFLAGS) -o hw_02_test -Iinclude $< obj/*

obj/%.o: src/%.cpp include/*.h obj
	$(CXX) $(CXXFLAGS) -c -o $@ -Iinclude $<





clean:
	rm -rf obj hw_02 hw_02_test