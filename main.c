#include "raylib.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>



char rom_name[] = "rom.ch8";
bool COSMAC_VIP_1977 = true;
int cycles_per_frame = 10;
int FONT_START_ADDRESS = 0x50;
int beep_frequency = 440;
bool debug = false;


#define CHIP8_WIDTH 64
#define CHIP8_HEIGHT 32
#define PIXEL_SCALE 10 
#define CPU_CYCLE_PER_FRAME 12

#define X(opcode)   ((opcode & 0x0F00) >> 8)
#define Y(opcode)   ((opcode & 0x00F0) >> 4)
#define N(opcode)   (opcode & 0x000F) 
#define NN(opcode)  (opcode & 0x00FF)    
#define NNN(opcode) (opcode & 0x0FFF)

void draw();
bool LoadROM(const char* filename);
void EmulateCycle();
void UpdateKeys();
void GenerateBeep();
void Op00E0();
void Op1NNN();
void Op6XNN();
void Op7XNN();
void OpANNN();
void OpDXYN();
void Op2NNN();
void Op00EE();
void Op3XNN();
void Op4XNN();
void Op5XY0();
void Op9XY0();

void Op8XY0();
void Op8XY1();
void Op8XY2();
void Op8XY3();
void Op8XY4();
void Op8XY5();
void Op8XY6();
void Op8XY7();
void Op8XYE();

void OpFX07();
void OpFX15();
void OpFX18();

void OpEX9E();
void OpEXA1();

void OpFX0A();
void OpFX1E();
void OpFX29();
void OpFX33();
void OpFX55();
void OpFX65();

void OpCXNN();
void OpBNNN();

uint16_t opcode;

uint8_t memory[4096];
uint8_t V[16];
uint16_t I;
uint16_t pc;

uint16_t stack[16];
uint8_t sp;

uint8_t delay_timer = 0;
uint8_t sound_timer = 0;

uint8_t fontset[80] = {
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

bool display[CHIP8_HEIGHT][CHIP8_WIDTH];
bool keys[16];

float accumulator = 0.0f;
const float TIME_STEP = 1.0f / 60.0f;

int main()
{
	srand(time(NULL));

	if (!LoadROM(rom_name))
	{
		return 1;
	}

	pc = 0x200;

	for (int i = 0; i < 80; i++) {
		memory[i + FONT_START_ADDRESS] = fontset[i];
	}

	InitWindow(CHIP8_WIDTH * PIXEL_SCALE, CHIP8_HEIGHT * PIXEL_SCALE, "CHIP-8 Emulator");
	SetTargetFPS(60);

	InitAudioDevice();
	SetAudioStreamBufferSizeDefault(4096);
	AudioStream beep = LoadAudioStream(44100, 16, 1);
	SetAudioStreamCallback(beep, GenerateBeep);


	while (!WindowShouldClose())
	{
		UpdateKeys();

		accumulator += GetFrameTime();

		while (accumulator >= TIME_STEP)
		{
			for (int i = 0; i < CPU_CYCLE_PER_FRAME; i++)
			{
				EmulateCycle();
			}

			if (delay_timer > 0)
			{
				delay_timer--;
			}

			if (sound_timer > 0)
			{
				sound_timer--;
				if (!IsAudioStreamPlaying(beep))
				{
					PlayAudioStream(beep);
				}
			}
			else
			{
				if (IsAudioStreamPlaying(beep))
				{
					StopAudioStream(beep);
				}
			}


			accumulator -= TIME_STEP;
		}

		PlayAudioStream(beep);
		draw();
		if (debug == true)
		{
			DrawFPS(180, 10);
		}
	}

	CloseWindow();
	return 0;

}

void draw() {
	BeginDrawing();
	ClearBackground(BLACK);

	for (int y = 0; y < CHIP8_HEIGHT; y++) {
		// ZDE OPRAVENO NA CHIP8_WIDTH
		for (int x = 0; x < CHIP8_WIDTH; x++) {

			// Zápis musí být striktnì [y][x]
			if (display[y][x] == true) {
				DrawRectangle(x * PIXEL_SCALE, y * PIXEL_SCALE, PIXEL_SCALE, PIXEL_SCALE, WHITE);
			}
		}
	}
	EndDrawing();
}

void UpdateKeys()
{
	keys[0x1] = IsKeyDown(KEY_ONE);
	keys[0x2] = IsKeyDown(KEY_TWO);
	keys[0x3] = IsKeyDown(KEY_THREE);
	keys[0xC] = IsKeyDown(KEY_FOUR);

	keys[0x4] = IsKeyDown(KEY_Q);
	keys[0x5] = IsKeyDown(KEY_W);
	keys[0x6] = IsKeyDown(KEY_E);
	keys[0xD] = IsKeyDown(KEY_R);

	keys[0x7] = IsKeyDown(KEY_A);
	keys[0x8] = IsKeyDown(KEY_S);
	keys[0x9] = IsKeyDown(KEY_D);
	keys[0xE] = IsKeyDown(KEY_F);

	keys[0xA] = IsKeyDown(KEY_Y);
	keys[0x0] = IsKeyDown(KEY_X);
	keys[0xB] = IsKeyDown(KEY_C);
	keys[0xF] = IsKeyDown(KEY_V);
}

bool LoadROM(const char* filename) {

	FILE* file = fopen(filename, "rb");
	if (file == NULL) {
		printf("ERROR: Unable to open file %s\n", filename);
		return false;
	}

	fseek(file, 0, SEEK_END);
	long file_size = ftell(file);
	fseek(file, 0, SEEK_SET);

	long max_size = 4096 - 512;
	if (file_size > max_size) {
		printf("ERROR: ROM to big \n");
		fclose(file);
		return false;
	}


	fread(&memory[0x200], 1, file_size, file);

	fclose(file);
	return true;
}

void EmulateCycle()
{
	opcode = (memory[pc] << 8) | memory[pc + 1];
	pc += 2;

	switch (((opcode & 0xF000) >> 12))
	{
	case 0x0:
		if (opcode == 0x00E0)
			Op00E0();

		if (opcode == 0x00EE)
			Op00EE();
		break;

	case 0x1:
		Op1NNN();
		break;

	case 0x2:
		Op2NNN();
		break;

	case 0x3:
		Op3XNN();
		break;

	case 0x4:
		Op4XNN();
		break;

	case 0x5:
		Op5XY0();
		break;

	case 0x6:
		Op6XNN();
		break;

	case 0x7:
		Op7XNN();
		break;

	case 0x8:
		switch (N(opcode))
		{
		case 0x0:
			Op8XY0();
			break;

		case 0x1:
			Op8XY1();
			break;

		case 0x2:
			Op8XY2();
			break;

		case 0x3:
			Op8XY3();
			break;

		case 0x4:
			Op8XY4();
			break;

		case 0x5:
			Op8XY5();
			break;

		case 0x6:
			Op8XY6();
			break;

		case 0x7:
			Op8XY7();
			break;

		case 0xE:
			Op8XYE();
			break;

		default:
			printf("ERROR: Unknow opcode in 8xxx section");
			break;
		}
		break;

	case 0x9:
		Op9XY0();
		break;

	case 0xA:
		OpANNN();
		break;

	case 0xB:
		OpBNNN();
		break;

	case 0xC:
		OpCXNN();
		break;

	case 0xD:
		OpDXYN();
		break;

	case 0xE:
		switch (NN(opcode))
		{
		case 0x9E:
			OpEX9E();
			break;

		case 0xA1:
			OpEXA1();
			break;

		default:
			printf("ERROR: Unknow opcode in Exxx section");
			break;

		}

		break;

	case 0xF:
		switch (NN(opcode))
		{
		case 0x07:
			OpFX07();
			break;

		case 0x15:
			OpFX15();
			break;

		case 0x18:
			OpFX18();
			break;

		case 0x0A:
			OpFX0A();
			break;

		case 0x1E:
			OpFX1E();
			break;

		case 0x29:
			OpFX29();
			break;

		case 0x33:
			OpFX33();
			break;

		case 0x55:
			OpFX55();
			break;

		case 0x65:
			OpFX65();
			break;


		default:
			printf("ERROR: Unknow opcode in 0xxx section");
			break;
		}

		break;

	default:
		printf("ERROR: Unknow opcode");

	}



}

void GenerateBeep(void* buffer, unsigned int frames)
{
	short* d = (short*)buffer;
	static int phase = 0;
	int amplitude = 4000;
	int sampleRate = 44100;

	for (unsigned int i = 0; i < frames; i++)
	{
		phase++;
		if (beep_frequency == 0) continue;

		if (phase >= sampleRate / beep_frequency) phase = 0;


		d[i] = (phase < (sampleRate / beep_frequency) / 2) ? amplitude : -amplitude;
	}
}

void Op00E0()
{
	for (int y = 0; y < CHIP8_HEIGHT; y++)
	{
		for (int x = 0; x < CHIP8_WIDTH; x++)
		{
			display[y][x] = false;
		}
	}

}

void Op1NNN()
{
	pc = NNN(opcode);
}

void Op6XNN()
{
	V[X(opcode)] = NN(opcode);
}

void Op7XNN()
{
	V[X(opcode)] += NN(opcode);
}

void OpANNN()
{
	I = NNN(opcode);

}

void OpDXYN()
{
	uint8_t start_x = V[X(opcode)] % CHIP8_WIDTH;
	uint8_t start_y = V[Y(opcode)] % CHIP8_HEIGHT;
	V[15] = 0;
	for (int y = 0; y < N(opcode); y++)
	{
		//if (y > 32)
		//{
		//	return;
		//}
		if (start_y + y >= CHIP8_HEIGHT)
		{
			break;
		}

		uint8_t sprite_byte = memory[I + y];

		for (int x = 0; x < 8; x++)
		{

			if (start_x + x >= CHIP8_WIDTH)
			{
				break;
			}

			bool sprite_pixel = (sprite_byte >> (7 - x)) & 1;

			if (sprite_pixel)
			{
				int screen_x = start_x + x;
				int screen_y = start_y + y;

				if (display[screen_y][screen_x] == true)
				{
					V[15] = 1;
					display[screen_y][screen_x] = false;
				}
				else
				{
					display[screen_y][screen_x] = true;
				}
			}

		}

	}
}

void Op2NNN()
{
	stack[sp] = pc;
	sp++;
	pc = NNN(opcode);

}

void Op00EE()
{
	sp--;
	pc = stack[sp];
}

void Op3XNN()
{
	if (V[X(opcode)] == NN(opcode))
		pc += 2;

}

void Op4XNN()
{
	if (V[X(opcode)] != NN(opcode))
		pc += 2;

}

void Op5XY0()
{
	if (V[X(opcode)] == V[Y(opcode)])
		pc += 2;

}

void Op9XY0()
{
	if (V[X(opcode)] != V[Y(opcode)])
		pc += 2;

}

void Op8XY0()
{
	V[X(opcode)] = V[Y(opcode)];
}

void Op8XY1()
{
	V[X(opcode)] = V[X(opcode)] | V[Y(opcode)];
}

void Op8XY2()
{
	V[X(opcode)] = V[X(opcode)] & V[Y(opcode)];
}

void Op8XY3()
{
	V[X(opcode)] = V[X(opcode)] ^ V[Y(opcode)];
}

void Op8XY4()
{
	if (V[X(opcode)] + V[Y(opcode)] > 255)
	{
		V[X(opcode)] = V[X(opcode)] + V[Y(opcode)] - 256;
		V[0xF] = 1;
	}
	else
	{
		V[X(opcode)] = V[X(opcode)] + V[Y(opcode)];
		V[0xF] = 0;
	}

}

void Op8XY5()
{
	if (V[X(opcode)] - V[Y(opcode)] < 0)
	{
		V[X(opcode)] = V[X(opcode)] - V[Y(opcode)] + 256;
		V[0xF] = 0;
	}
	else
	{
		V[X(opcode)] = V[X(opcode)] - V[Y(opcode)];
		V[0xF] = 1;
	}
}

void Op8XY6()
{
	/*if (COSMAC_VIP_1977)
	{
		V[X(opcode)] = V[Y(opcode)];
	}

	V[0xF] = V[X(opcode)] & 0x1;
	V[X(opcode)] = V[X(opcode)] >> 1;*/

	if (COSMAC_VIP_1977)
	{
		V[X(opcode)] = V[Y(opcode)];
	}

	uint8_t shifted_bit = V[X(opcode)] & 0x1;

	V[X(opcode)] = V[X(opcode)] >> 1;

	V[0xF] = shifted_bit;
}

void Op8XY7()
{
	if (V[Y(opcode)] - V[X(opcode)] < 0)
	{
		V[X(opcode)] = V[Y(opcode)] - V[X(opcode)] + 256;
		V[0xF] = 0;
	}
	else
	{
		V[X(opcode)] = V[Y(opcode)] - V[X(opcode)];
		V[0xF] = 1;
	}
}

void Op8XYE()
{
	/*if (COSMAC_VIP_1977)
	{
		V[X(opcode)] = V[Y(opcode)];
	}

	V[0xF] = V[X(opcode)] & 0x1;
	V[X(opcode)] = V[X(opcode)] << 1;*/

	if (COSMAC_VIP_1977)
	{
		V[X(opcode)] = V[Y(opcode)];
	}

	uint8_t shifted_bit = (V[X(opcode)] >> 7) & 0x1;

	V[X(opcode)] = V[X(opcode)] << 1;

	V[0xF] = shifted_bit;

}

void OpFX07()
{
	V[X(opcode)] = delay_timer;
}

void OpFX15()
{
	delay_timer = V[X(opcode)];
}

void OpFX18()
{
	sound_timer = V[X(opcode)];
}

void OpEX9E()
{
	if (keys[V[X(opcode)] & 0xF] == 1)
	{
		pc += 2;
	}
}

void OpEXA1()
{
	if (keys[V[X(opcode)] & 0xF] == 0)
	{
		pc += 2;
	}
}

void OpFX0A()
{
	for (int i = 0x0; i <= 0xF; i++)
	{
		if (keys[i])
		{
			pc = pc + 2;
			V[X(opcode)] = i;
			break;
		}
	}

	pc = pc - 2;
}

void OpFX1E()
{
	I = I + V[X(opcode)];
}

void OpFX29()
{
	I = V[X(opcode)] * 5 + FONT_START_ADDRESS;
}

void OpFX33()
{
	memory[I] = V[X(opcode)] / 100;
	memory[I + 1] = (V[X(opcode)] / 10) % 10;
	memory[I + 2] = (V[X(opcode)] % 100) % 10;
}

void OpFX55()
{
	for (int i = 0; i <= X(opcode); i++)
	{
		memory[I + i] = V[i];
	}

	if (COSMAC_VIP_1977)
	{
		I = I + X(opcode) + 1;
	}
}

void OpFX65()
{
	for (int i = 0; i <= X(opcode); i++)
	{
		V[i] = memory[I + i];
	}

	if (COSMAC_VIP_1977)
	{
		I = I + X(opcode) + 1;
	}
}

void OpCXNN()
{
	V[X(opcode)] = (rand() & NN(opcode));
}

void OpBNNN()
{
	if (COSMAC_VIP_1977)
	{
		pc = NNN(opcode) + V[0];
	}
	else
	{
		pc = NNN(opcode) + V[X(opcode)];
	}
}