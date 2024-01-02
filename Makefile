main: main.cpp chip8
	g++ -c main.cpp -g3 -O3
	g++ main.o chip8.o -o chip8 -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -g3 -O3
chip8: chip8.hpp chip8.cpp
	g++ -c chip8.cpp -g3 -O3
clean:
	rm main.o chip8.o chip8