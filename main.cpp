#include "chip8.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <map>
#include <iostream>
#include <vector>

bool key_pressed(uint8_t number)
{
	map<uint8_t, sf::Keyboard::Key> keys = {
		{1, sf::Keyboard::Key::Num1},
		{2, sf::Keyboard::Key::Num2},
		{3, sf::Keyboard::Key::Num3},
		{0xC, sf::Keyboard::Key::Num4},
		{4, sf::Keyboard::Key::Q},
		{5, sf::Keyboard::Key::W},
		{6, sf::Keyboard::Key::E},
		{0xD, sf::Keyboard::Key::R},
		{7, sf::Keyboard::Key::A},
		{8, sf::Keyboard::Key::S},
		{9, sf::Keyboard::Key::D},
		{0xE, sf::Keyboard::Key::F},
		{0xA, sf::Keyboard::Key::Z},
		{0, sf::Keyboard::Key::X},
		{0xB, sf::Keyboard::Key::C},
		{0xF, sf::Keyboard::Key::V},
	};
	return sf::Keyboard::isKeyPressed(keys.at(number));
}

uint8_t get_key()
{
	map<sf::Keyboard::Key, uint8_t> numbers = {
		{sf::Keyboard::Key::Num1, 1},
		{sf::Keyboard::Key::Num2, 2},
		{sf::Keyboard::Key::Num3, 3},
		{sf::Keyboard::Key::Num4, 0xC},
		{sf::Keyboard::Key::Q, 4},
		{sf::Keyboard::Key::W, 5},
		{sf::Keyboard::Key::E, 6},
		{sf::Keyboard::Key::R, 0xD},
		{sf::Keyboard::Key::A, 7},
		{sf::Keyboard::Key::S, 8},
		{sf::Keyboard::Key::D, 9},
		{sf::Keyboard::Key::F, 0xE},
		{sf::Keyboard::Key::Z, 0xA},
		{sf::Keyboard::Key::X, 0},
		{sf::Keyboard::Key::C, 0xB},
		{sf::Keyboard::Key::V, 0xF},
	};
	sf::Keyboard::Key pressed_key = sf::Keyboard::Key::Unknown;
	uint8_t number;
	while(pressed_key == sf::Keyboard::Key::Unknown)
	{
		for(auto [key, value]: numbers)
		{
			//cout << int(value) << endl;
			if(sf::Keyboard::isKeyPressed(key))
			{
				pressed_key = key;
				number = value;
				//cout << "KEY PRESSED: " << int(number) << endl;
			}
		}
	}
	while(sf::Keyboard::isKeyPressed(pressed_key));
	//cout << "KEY RELEASED" << endl;
	return number;
}

sf::Sound sound;
void play_sound()
{
	sound.play();
}

int main(int argc, char *argv[])
{
	if(argc < 2)
	{
		cerr << "Usage: " << argv[0] << " [rom] [-d]" << endl;
		exit(1);
	}
	chip8::Emulator emulator(key_pressed, get_key, play_sound, argc >= 3 && string(argv[2]) == "-d");
	emulator.load(argv[1]);
	vector<sf::Int16> samples;
	for(int i = 0; i < 7; i++)
	{
		for(int j = 0; j < 53; j++)
			samples.push_back(4096);
		for(int j = 0; j < 53; j++)
			samples.push_back(-4096);
	}
	sf::SoundBuffer buffer;
	buffer.loadFromSamples(&samples[0], samples.size(), 1, 44100);
	sound.setBuffer(buffer);
	sf::RenderWindow window(sf::VideoMode(64, 32), "Chip-8");
	sf::Clock clock;
	while(window.isOpen())
	{
		sf::Event event;
		while(window.pollEvent(event))
		{
			if(event.type == sf::Event::Closed)
				window.close();
		}
		clock.restart();
		while(clock.getElapsedTime().asMicroseconds() < 16667)
			emulator.step();
		sf::Image image;
		image.create(64, 32);
		auto screen = emulator.get_screen();
		for(int y = 0; y < 32; y++)
		{
			for(int x = 0; x < 64; x++)
				image.setPixel(x, y, screen.at(y).at(x) ? sf::Color::White : sf::Color::Black);
		}
		sf::Texture texture;
		texture.create(64, 32);
		texture.update(image);
		sf::Sprite sprite;
		sprite.setTexture(texture);
		window.draw(sprite);
		window.display();
	}
	return 0;
}