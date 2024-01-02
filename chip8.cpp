// http://devernay.free.fr/hacks/chip8/C8TECH10.HTM

#include "chip8.hpp"
#include <exception>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <iterator>
#include <iostream>

namespace chip8
{
	class InvalidInstructionError: public exception
	{
		uint16_t pc, instruction;
		string message;
		public:
			InvalidInstructionError(uint16_t pc, uint16_t instruction) noexcept: pc(pc), instruction(instruction)
			{
				stringstream stream;
				stream << "Invalid instruction at " << setw(4) << setfill('0') << right << hex << pc << ": " << setw(4) << setfill('0') << right << hex << instruction;
				message = stream.str();
			}
			virtual const char *what() const noexcept {return message.c_str();}
	};
	Emulator::Emulator(function<bool(uint8_t)> key_pressed, function<uint8_t()> get_key, function<void()> play_sound, bool debug): pc(0x200), gen(rd()), distrib(0, 255), key_pressed(key_pressed), get_key(get_key), play_sound(play_sound), last_dt_tick(chrono::steady_clock::now()), last_st_tick(chrono::steady_clock::now()), debug(debug)
	{
		for(int y = 0; y < 32; y++)
		{
			for(int x = 0; x < 64; x++)
				screen.at(y).at(x) = false;
		}
	}
	uint8_t Emulator::read8(uint16_t address) {return memory.at(address);}
	uint16_t Emulator::read16(uint16_t address) {return memory.at(address) * 256 + memory.at(address + 1);}
	void Emulator::write8(uint16_t address, uint8_t data) {memory.at(address) = data;}
	array<array<bool, 64>, 32> Emulator::get_screen() {return screen;}
	void Emulator::load(string filename)
	{
		ifstream rom(filename, ios::binary);
		rom.read(reinterpret_cast<char *>(memory.data()) + 0x200, 0x1000 - 0x200);
	}
	void Emulator::step()
	{
		int since_last_dt_tick = chrono::duration_cast<chrono::microseconds>(chrono::steady_clock::now() - last_dt_tick).count();
		int since_last_st_tick = chrono::duration_cast<chrono::microseconds>(chrono::steady_clock::now() - last_st_tick).count();
		//cout << since_last_dt_tick << " " << int(dt) << endl;
		if(since_last_dt_tick > 16 && dt > 0)
		{
			dt = max(0, dt - since_last_dt_tick / 16);
			//cout << since_last_dt_tick << " " << int(dt) << endl;
			last_dt_tick = chrono::steady_clock::now();
		}
		if(since_last_st_tick > 16 && st > 0)
		{
			st = max(0, st - since_last_st_tick / 16);
			last_st_tick = chrono::steady_clock::now();
			play_sound();
		}
		uint16_t instruction = read16(pc);
		if(debug)
		{
			cout << "PC: " << setw(4) << setfill('0') << right << hex << pc << " M[PC]: " << setw(4) << setfill('0') << right << hex << instruction << " I: " << setw(4) << setfill('0') << right << hex << i << endl;
			for(int m = 0; m < 16; m += 4)
			{
				cout << "V" << hex << m << "-" << hex << (m + 3) << ": ";
				for(int n = 0; n < 4; n++)
					cout << setw(4) << setfill('0') << right << hex << int(v.at(m + n)) << " ";
				cout << endl;
			}
			getchar();
		}
		pc += 2;
		uint8_t &vx = v.at(instruction >> 8 & 0xF);
		uint8_t &vy = v.at(instruction >> 4 & 0xF);
		uint16_t addr = instruction & 0xFFF;
		uint8_t byte = instruction & 0xFF;
		uint8_t nibble = instruction & 0xF;
		switch(instruction >> 12)
		{
			case 0:
				switch(byte)
				{
					case 0xE0:
						// CLS
						for(int y = 0; y < 32; y++)
						{
							for(int x = 0; x < 64; x++)
								screen.at(y).at(x) = false;
						}
						break;
					case 0xEE:
						// RET
						pc = stack.at(sp--);
						break;
					default:
						throw InvalidInstructionError(pc - 2, instruction);
				}
				break;
			case 1:
				// JP addr
				pc = addr;
				break;
			case 2:
				// CALL addr
				stack.at(++sp) = pc;
				pc = addr;
				break;
			case 3:
				// SE Vx, byte
				if(vx == byte)
					pc += 2;
				break;
			case 4:
				// SNE Vx, byte
				if(vx != byte)
					pc += 2;
				break;
			case 5:
				// SE Vx, Vy
				if(vx == vy)
					pc += 2;
				break;
			case 6:
				// LD Vx, byte
				vx = byte;
				break;
			case 7:
				// ADD Vx, byte
				vx += byte;
				break;
			case 8:
				int flag;
				switch(nibble)
				{
					case 0:
						// LD Vx, Vy
						flag = 0;
						vx = vy;
						break;
					case 1:
						// OR Vx, Vy
						flag = 0;
						vx |= vy;
						break;
					case 2:
						// AND Vx, Vy
						flag = 0;
						vx &= vy;
						break;
					case 3:
						// XOR Vx, Vy
						flag = 0;
						vx ^= vy;
						break;
					case 4:
						// ADD Vx, Vy
						flag = vx + vy > 255;
						vx += vy;
						break;
					case 5:
						// SUB Vx, Vy
						flag = vx >= vy;
						vx -= vy;
						break;
					case 6:
						// SHR Vx, Vy
						flag = vy & 1;
						vx = vy >> 1;
						break;
					case 7:
						// SUBN Vx, Vy
						flag = vy >= vx;
						vx = vy - vx;
						break;
					case 0xE:
						// SHL Vx, Vy
						flag = vy >> 7;
						vx = vy << 1;
						break;
					default:
						throw InvalidInstructionError(pc - 2, instruction);
				}
				v.at(0xF) = flag;
				break;
			case 9:
				// SNE Vx, Vy
				if(vx != vy)
					pc += 2;
				break;
			case 0xA:
				// LD I, addr
				i = addr;
				break;
			case 0xB:
				// JP V0, addr
				pc = addr + v.at(0);
				break;
			case 0xC:
				// RND Vx, byte
				vx = distrib(gen) & byte;
				break;
			case 0xD:
				// DRW Vx, Vy, nibble
				v.at(0xF) = 0;
				for(int y = 0; y < nibble; y++)
				{
					uint8_t sprite_byte = read8(i + y);
					for(int x = 0; x < 8; x++)
					{
						if(vx % 64 + x < 64 && vy % 32 + y < 32)
						{
							if(screen.at(vy % 32 + y).at(vx % 64 + x) && (sprite_byte >> (7 - x)) & 1)
								v.at(0xF) = 1;
							screen.at(vy % 32 + y).at(vx % 64 + x) ^= (sprite_byte >> (7 - x)) & 1;
						}
					}
				}
				break;
			case 0xE:
				switch(byte)
				{
					case 0x9E:
						// SKP Vx
						if(key_pressed(vx))
							pc += 2;
						break;
					case 0xA1:
						// SKNP Vx
						if(!key_pressed(vx))
							pc += 2;
						break;
					default:
						throw InvalidInstructionError(pc - 2, instruction);
				}
				break;
			case 0xF:
				switch(byte)
				{
					case 0x07:
						// LD Vx, DT
						vx = dt;
						break;
					case 0x0A:
						// LD Vx, K
						vx = get_key();
						break;
					case 0x15:
						// LD DT, Vx
						dt = vx;
						break;
					case 0x18:
						// LD ST, Vx
						st = vx;
						break;
					case 0x1E:
						// ADD I, Vx
						i += vx;
						break;
					case 0x29:
						// LD F, Vx
						i = (vx & 0xF) * 5;
						break;
					case 0x33:
						// LD B, Vx
						write8(i, vx / 100);
						write8(i + 1, vx / 10 % 10);
						write8(i + 2, vx % 10);
						break;
					case 0x55:
						// LD [I], Vx
						for(int x = 0; x <= (instruction >> 8 & 0xF); x++)
							write8(i + x, v.at(x));
						break;
					case 0x65:
						// LD Vx, [I]
						for(int x = 0; x <= (instruction >> 8 & 0xF); x++)
							v.at(x) = read8(i + x);
						break;
					default:
						throw InvalidInstructionError(pc - 2, instruction);
				}
				break;
		}
	}
}