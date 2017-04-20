all: main.x

main.x: main.cpp wav.o
	g++ main.cpp wav.o -o main.x -lasound -std=c++0x

wav.o: wav.cpp wav.h
	g++ wav.cpp -c -o wav.o -lasound -std=c++0x
