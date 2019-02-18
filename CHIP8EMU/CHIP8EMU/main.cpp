#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "chip8.h"


sf::SoundBuffer bleepSound;
sf::Sound sound;
bool bleepLoaded = false;

void OnBleep()
{
	if (bleepLoaded)
	{
		sound.play();
	}
}

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		printf(argv[1]);
		printf("No Game Rom! Please drag a .ch8 file onto this .exe... - http://richard.io");
		sf::sleep(sf::Time(sf::seconds(10)));
		return 0;
	}
	
	// load bleep sound.
	if (!bleepSound.loadFromFile("bleep.wav"))
	{
		printf("No bleep.wav found! Please place bleep.wav next to the .exe or wait 3 more seconds... http://richardbamford.io");
		sf::sleep(sf::Time(sf::seconds(5)));
		bleepLoaded = false;
	}
	else
	{
		sound.setBuffer(bleepSound);
		sound.setVolume(50);
		bleepLoaded = true;
	}
	
	chip8 Chip8Emu;
	Chip8Emu.LoadGame(argv[1]);
	Chip8Emu.SetOnBleepCallback(OnBleep);

	int pixelSize = 32;
	sf::RectangleShape graphicRects[64 * 32];

	for (int y = 0; y < 32; y++)
	{
		for (int x = 0; x < 64; x++)
		{
			int index = x + (y * 64);
			graphicRects[index] = sf::RectangleShape(sf::Vector2f(pixelSize, pixelSize));
			graphicRects[index].setPosition(1 + (x * pixelSize), 1 + (y * pixelSize));
		}
	}

	sf::RenderWindow window(sf::VideoMode(pixelSize * 64, pixelSize * 32), "CHIP-8 EMU");

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
			{
				window.close();
			}

			if (event.type == sf::Event::KeyPressed)
			{
				if (event.key.code == sf::Keyboard::Num1)
				{
					Chip8Emu.SetKeys(1);
				}

				if (event.key.code == sf::Keyboard::Num2)
				{
					Chip8Emu.SetKeys(2);
				}

				if (event.key.code == sf::Keyboard::Num3)
				{
					Chip8Emu.SetKeys(3);
				}

				if (event.key.code == sf::Keyboard::Num4)
				{
					Chip8Emu.SetKeys(0xC);
				}

				if (event.key.code == sf::Keyboard::Q)
				{
					Chip8Emu.SetKeys(4);
				}

				if (event.key.code == sf::Keyboard::W)
				{
					Chip8Emu.SetKeys(5);
				}

				if (event.key.code == sf::Keyboard::E)
				{
					Chip8Emu.SetKeys(6);
				}

				if (event.key.code == sf::Keyboard::R)
				{
					Chip8Emu.SetKeys(0xD);
				}

				if (event.key.code == sf::Keyboard::A)
				{
					Chip8Emu.SetKeys(7);
				}

				if (event.key.code == sf::Keyboard::S)
				{
					Chip8Emu.SetKeys(8);
				}

				if (event.key.code == sf::Keyboard::D)
				{
					Chip8Emu.SetKeys(9);
				}

				if (event.key.code == sf::Keyboard::F)
				{
					Chip8Emu.SetKeys(0xE);
				}
				if (event.key.code == sf::Keyboard::Z)
				{
					Chip8Emu.SetKeys(0xA);
				}

				if (event.key.code == sf::Keyboard::X)
				{
					Chip8Emu.SetKeys(0);
				}

				if (event.key.code == sf::Keyboard::C)
				{
					Chip8Emu.SetKeys(0xB);
				}

				if (event.key.code == sf::Keyboard::V)
				{
					Chip8Emu.SetKeys(0xF);
				}
			}

			if (event.type == sf::Event::KeyReleased)
			{
				if (event.key.code == sf::Keyboard::Num1)
				{
					Chip8Emu.UnsetKeys(1);
				}

				if (event.key.code == sf::Keyboard::Num2)
				{
					Chip8Emu.UnsetKeys(2);
				}

				if (event.key.code == sf::Keyboard::Num3)
				{
					Chip8Emu.UnsetKeys(3);
				}

				if (event.key.code == sf::Keyboard::Num4)
				{
					Chip8Emu.UnsetKeys(0xC);
				}

				if (event.key.code == sf::Keyboard::Q)
				{
					Chip8Emu.UnsetKeys(4);
				}

				if (event.key.code == sf::Keyboard::W)
				{
					Chip8Emu.UnsetKeys(5);
				}

				if (event.key.code == sf::Keyboard::E)
				{
					Chip8Emu.UnsetKeys(6);
				}

				if (event.key.code == sf::Keyboard::R)
				{
					Chip8Emu.UnsetKeys(0xD);
				}

				if (event.key.code == sf::Keyboard::A)
				{
					Chip8Emu.UnsetKeys(7);
				}

				if (event.key.code == sf::Keyboard::S)
				{
					Chip8Emu.UnsetKeys(8);
				}

				if (event.key.code == sf::Keyboard::D)
				{
					Chip8Emu.UnsetKeys(9);
				}

				if (event.key.code == sf::Keyboard::F)
				{
					Chip8Emu.UnsetKeys(0xE);
				}
				if (event.key.code == sf::Keyboard::Z)
				{
					Chip8Emu.UnsetKeys(0xA);
				}

				if (event.key.code == sf::Keyboard::X)
				{
					Chip8Emu.UnsetKeys(0);
				}

				if (event.key.code == sf::Keyboard::C)
				{
					Chip8Emu.UnsetKeys(0xB);
				}

				if (event.key.code == sf::Keyboard::V)
				{
					Chip8Emu.UnsetKeys(0xF);
				}
			}
		}

		Chip8Emu.EmulateCycle();

		if (Chip8Emu.DrawFlag)
		{
			for (int y = 0; y < 32; y++)
			{
				for (int x = 0; x < 64; x++)
				{
					int index = x + (y * 64);
					if (Chip8Emu.GetPixel(x, y) == 1)
					{
						graphicRects[index].setFillColor(sf::Color(255, 255, 255, 255));
					}
					else
					{
						graphicRects[index].setFillColor(sf::Color(0, 0, 0, 255));
					}
				}
			}

			Chip8Emu.DrawFlag = false;
		}

		window.clear();
		for (int i = 0; i < 32 * 64; i++)
		{
			window.draw(graphicRects[i]);
		}
		window.display();

		//sf::sleep(sf::Time(sf::seconds(1.0f/60.0f)));
	}

	return 0;
}