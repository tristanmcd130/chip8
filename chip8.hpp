#include <array>
#include <cstdint>
#include <stack>
#include <functional>
#include <random>
#include <chrono>

using namespace std;

namespace chip8
{
	class Emulator
	{
		array<uint8_t, 4096> memory;
		array<uint8_t, 16> v;
		uint16_t i, pc;
		uint8_t dt, st;
		stack<uint16_t> call_stack;
		array<array<bool, 64>, 32> screen;
		random_device rd;
		mt19937 gen;
		uniform_int_distribution<uint8_t> distrib;
		function<bool(uint8_t)> key_pressed;
		function<uint8_t()> get_key;
		function<void()> play_sound;
		chrono::time_point<chrono::steady_clock> last_dt_tick, last_st_tick;
		bool debug;
		public:
			Emulator(function<bool(uint8_t)> key_pressed, function<uint8_t()> get_key, function<void()> play_sound, bool debug = false);
			uint8_t read8(uint16_t address);
			uint16_t read16(uint16_t address);
			void write8(uint16_t address, uint8_t data);
			array<array<bool, 64>, 32> get_screen();
			void load(string filename);
			void step();
	};
}