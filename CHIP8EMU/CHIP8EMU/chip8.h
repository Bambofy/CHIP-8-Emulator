#pragma once

#include <cstdlib>
#include <cstdio>

class chip8 {
public:
	chip8();
	~chip8();
	void LoadGame(const char* pGameName);
	void EmulateCycle();
	void SetKeys(unsigned char pKeyCode)
	{
		_key[pKeyCode] = 1;
	}
	void UnsetKeys(unsigned char pKeyCode)
	{
		_key[pKeyCode] = 0;
	}
	unsigned char GetPixel(int pX, int pY)
	{
		int index = pX + (pY * 64);
		return _gfx[index];
	}

	void SetOnBleepCallback(void (*pBleepFunction)(void))
	{
		_bleepFunction = pBleepFunction;
	}

	bool DrawFlag; // signal to re-draw the screen.
private:
	void (*_bleepFunction)(void) = nullptr;

	void Draw(unsigned char pVx, unsigned char pVy, unsigned char pHeight);

	unsigned char _memory[4096]; // memory

	unsigned char _V[16]; // cpu registers

	unsigned short _I; // index register (16bit/2byte)

	unsigned short _pc; // program counter

	// graphics pixels (64 x 32)
	unsigned char _gfx[64 * 32];

	// 60hz timers.
	unsigned char _delayTimer;

	unsigned char _soundTimer;

	unsigned short _stack[16];

	// stack pointer
	unsigned short _sp;

	// 0x0 - 0xF key states.
	unsigned char _key[16];

	unsigned short _opcode;

	unsigned char _fontset[80] =
	{
	  0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	  0x20, 0x60, 0x20, 0x20, 0x70, // 1
	  0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	  0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	  0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	  0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	  0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	  0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	  0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	  0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	  0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	  0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	  0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	  0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	  0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	  0xF0, 0x80, 0xF0, 0x80, 0x80  // F
	};

	bool _halt = false;
	unsigned char _haltReturnRegister;
};

chip8::chip8()
{
	_pc = 0x200; // program counter starts at 0x200. (after the interpreter)
	_opcode = 0;
	_I = 0;
	_sp = 0;

	// 0 graphics
	for (int i = 0; i < 64 * 32; i++) _gfx[i] = 0; // clear the screen.

	// 0 stack
	for (int i = 0; i < 16; i++) _stack[i] = 0; // clear the stack.

	// 0 registers
	for (int i = 0; i < 16; i++) _V[i] = 0; // clear the registers.

	// set memory to 0
	for (int i = 0; i < 4096; i++) _memory[i] = 0; // clear the memory.

	// fontset loading.
	for (int i = 0; i < 80; i++) _memory[i] = _fontset[i]; // first 80 bytes in memory are the fonts.

	// set keys buffer to 0s
	for (int i = 0; i < 16; i++) _key[i] = 0; // set memory to 0.

	_delayTimer = 0;
	_soundTimer = 0;

	printf("Loaded CHIP8 Emulator.\n");
}

chip8::~chip8()
{
}

void chip8::LoadGame(const char* pGameName)
{
	printf("Loading game: %s\n", pGameName);

	FILE* gameFile;
	gameFile = fopen(pGameName, "rb");

	if (gameFile != NULL)
	{
		// find the end of the file
		fseek(gameFile, 0, SEEK_END);

		long int fileSize = ftell(gameFile);

		char fileSizeString[512]; // convert to a string
		_itoa(fileSize, fileSizeString, 10);
		printf(fileSizeString);
		printf("\n");

		rewind(gameFile);

		char* fileBuffer = (char*)malloc(fileSize * sizeof(unsigned char));
		fread(fileBuffer, 1, fileSize, gameFile);
		for (int i = 0; i < fileSize; i++)		// load the file contents into memory.
		{
			_memory[512 + i] = fileBuffer[i];
		}
		free(fileBuffer);

		fclose(gameFile);
	}
}

void chip8::EmulateCycle()
{
	if (_halt)
	{
		// wait for key press
		bool keyDown = false;
		int keyCode = -1;
		for (int i = 0; i < 16; i++)
		{
			if (_key[i] != 0)
			{
				keyDown = true;
				keyCode = i;
			}
		}

		if (keyDown)
		{
			_halt = false;

			// key pressed stored in Vx
			_V[_haltReturnRegister] = (unsigned char)keyCode;
		}

		return;
	}

	// loading opcode  `0xA2F0` from memory, at the location defined in the PC.
	// memory[pc] = 0xA2 (1 byte)
	// memory[pc + 1] = 0xF0 (1 byte)

	_opcode = (_memory[_pc] << 8) | _memory[_pc + 1]; // 2 bytes wide. Big endian.

	unsigned char X = (_opcode & 0x0F00) >> 8;
	unsigned char Y = (_opcode & 0x00F0) >> 4;
	unsigned short NNN = (_opcode & 0x0FFF);
	unsigned char NN = (_opcode & 0x00FF);
	unsigned char N = (_opcode & 0x000F);

	_pc += 2;

	switch (_opcode & 0xF000) // msb = F
	{
	case 0x0000:
		switch (_opcode & 0x000F)
		{
		case 0x0000:
			// 00E0 (display clear)
			for (int i = 0; i < 64 * 32; i++) _gfx[i] = 0; // clear the screen.
			DrawFlag = true;
			break;
		case 0x000E:
			// 00EE (return from subroutine).
			_sp--;
			_pc = _stack[_sp];
			break;
		}
		break;
	case 0x1000:
		// (goto NNN)
		_pc = NNN;
		break;
	case 0x2000:
		// (*0xNNN)() call function at NNN.
		_stack[_sp] = _pc;
		_sp++;
		_pc = NNN;
		break;
	case 0x3000:
		// 3xNN - SE Vx, byte

		// (if Vx == NN) skip next instruction
		if (_V[X] == NN)
		{
			_pc += 2;
		}
		break;
	case 0x4000:
		// 4xNN - SNE Vx, Byte
		if (_V[X] != NN)
		{
			_pc += 2;
		}
		break;
	case 0x5000:
		// 5xy0 - SE Vx, Vy
		if (_V[X] == _V[Y])
		{
			_pc += 2;
		}
		break;
	case 0x6000:
		// LD Vx, byte
		// set Vx to NN
		_V[X] = NN;
		break;
	case 0x7000:
		// vx + nn (carry flag ignored)
		// 7xNN
		_V[X] += NN;

		break;
	case 0x8000:
		switch (_opcode & 0x000F) // check lsb 0-E
		{
		case 0x0000:
			// Vx = Vy
			// 8xy0 = LD Vx, Vy
			_V[X] = _V[Y];
			break;
		case 0x0001:
			// Vx = Vx | Vy
			// 8xy1 = OR Vx, Vy
			_V[X] |= _V[Y];
			break;
		case 0x0002:
			// Vx = Vx & Vy
			// 8xy2 - Vx = Vx AND Vy
			_V[X] &= _V[Y];
			break;
		case 0x0003:
			// Vx = Vx ^ Vy
			// 8xy3 XOR Vx, Vy
			_V[X] ^= _V[Y];
			break;
		case 0x0004:
			// Vx += Vy
			// 8xy4 - ADD Vx, Vy

			if ((_V[X] + _V[Y]) > 0x00FF)
			{
				_V[0x000F] = 1; // carry
			}
			else
			{
				_V[0x000F] = 0;
			}

			_V[X] += _V[Y];
			break;
		case 0x0005:
			// Vx -= Vy
			// 8xy5 - Sub Vx, Vy
			// Vx = Vx - Vy
			// Vf = NOT borrow
			if (_V[X] > _V[Y])
			{
				_V[0x000F] = 1; // borrow
			}
			else
			{
				_V[0x000F] = 0; // no borrow
			}

			_V[X] -= _V[Y];
			break;
		case 0x0006:
			// Vx >> = 1 (stores the lsb of Vx in Vf) and shifts Vx to the right by 1
			// 8xy6 - SHR Vx {, Vy}
			_V[0x000F] = (_V[X] & 0b00000001); // what is Vy is supplied?

			_V[X] >>= 1;
			break;
		case 0x0007:
			// Vx = vy - vx
			// 8xy7
			if (_V[Y] > _V[X])
			{
				_V[0x000F] = 1; // borrow
			}
			else
			{
				_V[0x000F] = 0;
			}

			_V[X] = _V[Y] - _V[X];
			break;
		case 0x000E:
			// 8xyE - SHL Vx {, Vy}

			// Vx <<= 1 (stores the msb of Vx in Vf) and shifts Vx to the left by 1
			_V[0x000F] = _V[X] & 0b10000000; // _V[x] is 8 bits wide, 1 byte.
			_V[X] <<= 1;
			break;
		}
		break;
	case 0x9000:
		// 9xy0 - SNE Vx, Vy
		// condition (if Vx != Vy)
		if (_V[X] != _V[Y])
		{
			_pc += 2;
		}
		break;
	case 0xA000:
		// sets i to the address NNN.
		// LD I, addr
		_I = NNN;
		break;
	case 0xB000:
		// JP V0, addr
		// PC = V0 + NNN
		_pc = _V[0] + NNN;
		break;
	case 0xC000:
		// random Vx = rand() & NN
		_V[X] = (unsigned char)((rand() % 255)) & NN;
		break;
	case 0xD000:
		// draw(Vx, Vy, N)
		Draw(_V[X], _V[Y], N);
		break;
	case 0xE000:
		switch (_opcode & 0x00FF)
		{
		case 0x009E:
			// Ex9E - SKP Vx
			// if (key() == Vx)
			if (_key[_V[X]] == 1)
			{
				_pc += 2;
			}
			break;
		case 0x00A1:
			// ExA1 - SKPN Vx
			if (_key[_V[X]] == 0)
			{
				_pc += 2;
			}
			break;
		}
		break;
	case 0xF000:
		switch (_opcode & 0x00FF)
		{
		case 0x0007:
			// Fx07 - LD Vx, DT
			// Vx = getdelay();
			_V[X] = _delayTimer;
			break;
		case 0x000A:
			// Fx0A - LD Vx, K
			// vx =  wait for key press
			_halt = true;
			_haltReturnRegister = X;
			break;
		case 0x0015:
			// Fx15 - LD DT, Vx
			// set delay timer to Vx
			_delayTimer = _V[X];
			break;
		case 0x0018:
			// Fx18 - LD ST, Vx
			// set sound timer to vX
			_soundTimer = _V[X];
			break;
		case 0x001E:
			// add Vx to I.
			// Fx1E - ADD I, Vx
			_I += _V[X];
			break;
		case 0x0029:
			// Fx29 - LD F, Vx
			// set I to location of the sprite.
			// Vx contains a character (0 to F)
			// represented by 4x5 font.

			// !! Vx is the row index !!
			// 5 is the stride.
			_I = _V[X] * 5;
			break;
		case 0x0033:
			// Fx33 - LD B, Vx
			// set_BCD(Vx);
			// _V[x] is unsigned char, 4 bytes.
			_memory[_I] = (_V[X]) / 100;
			_memory[_I + 1] = (_V[X] / 10) % 10;
			_memory[_I + 2] = (_V[X] / 100) % 10;
			break;
		case 0x0055:
			// Fx55 - LD [i], Vx
			// _V[1] = 255 here.
			// stores V0->Vx registers to memory starting at addres I
			// reg_dump()

			for (int i = 0; i <= X; ++i)
			{
				_memory[_I + i] = _V[i];
			}
			break;
		case 0x0065:
			// reg_load()
			for (int i = 0; i <= X; i++)
			{
				_V[i] = _memory[_I + i];
			}
			break;
		}
		break;
	}

	if (_delayTimer > 0)
	{
		_delayTimer--;
	}

	if (_soundTimer > 0)
	{
		if (_soundTimer == 1)
		{
			if (_bleepFunction != nullptr)
			{
				(*_bleepFunction)();
			}
		}
		_soundTimer--;
	}

}


void chip8::Draw(unsigned char pX, unsigned char pY, unsigned char pHeight)
{
	// draw a sprite at vx, vy.
	_V[0x000F] = 0;

	for (int yLine = 0; yLine < pHeight; yLine++)
	{
		unsigned short fontPixelRow = _memory[_I + yLine]; // _I represents the row index. opcode = FX1E

		for (int xLine = 0; xLine < 8; xLine++)
		{
			int x = pX + xLine;
			int y = pY + yLine;

			if (y > 31)
			{
				y = (pY + yLine) - 32;
			}
			if (x > 63)
			{
				x = (pX + xLine) - 64;
			}

			// 0x80 = 1000 0000

			// WE ARE CONVERTING EACH BIT INTO A PIXEL!!

			// if any bit in this MEMORY/RAM row is 1
			if ((fontPixelRow & (0x80 >> xLine)) != 0) // only XOR values where the font actually draws them.
			{
				// gfx[x] is either 1 or 0.
				if (_gfx[(x + (y * 64))] == 1)
				{
					_V[0x000F] = 1;
				}
				_gfx[(x + (y * 64))] ^= 1;
			}
		}
	}

	DrawFlag = true;
}