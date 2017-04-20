all: main.x

main.x: main.cpp wav.o
	g++ main.cpp -o main.x -lasound

wav.o: wav.cpp wav.h
	g++ wav.cpp -c -o wav.o -lasound

