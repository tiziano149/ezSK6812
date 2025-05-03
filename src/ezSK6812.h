#include <Arduino.h>
#include <SPI.h>

class ezSK6812
{
public:
	ezSK6812(uint32_t num_leds, SPIClass &SPI_peripheral = SPI1) : num_leds(num_leds), SPI_peripheral(SPI_peripheral)
	{
		;
	}

	void begin()
	{
		this->SPI_peripheral.begin();
		SPISettings settings(8000000, MSBFIRST, SPI_MODE1);
		this->SPI_peripheral.beginTransaction(settings);
	}

	void end()
	{
		this->SPI_peripheral.endTransaction();
		this->SPI_peripheral.end();
	}

	void set_pixel(uint8_t red, uint8_t green, uint8_t blue, uint8_t white, uint8_t brightness = 100)
	{
		// Assign the colors in the output array in GRBW order
		uint8_t colors[4] = {green, red, blue, white};

		// If brightness is not 100% we take away from the color intensity of each component proportionally
		if (brightness > 0 && brightness <= 100)
		{
			for (uint8_t c = 0; c < 4; c++)
			{
				uint32_t color_adj = (uint32_t)colors[c] * brightness / 100;
				colors[c] = (uint8_t)color_adj;
			}
		}
		else if (brightness == 0)
		{
			// If brightness is zero - set all color components to zero
			memset(colors, 0, sizeof(colors));
		}

		// Go through the output array
		for (uint8_t c = 0; c < 4; c++)
		{
			// Go through the bits of each byte
			for (int i = 8; i > 0; i--)
			{
				// If the current bit is set send a 1, otherwise send a 0
				uint8_t curr = ((colors[c] >> (i - 1)) & 1);
				if (curr)
				{
					this->one();
				}
				else
				{
					this->zero();
				}
			}
		}
	}

	// Allows individual LEDs to be set to different colors
	void set_pixel(uint32_t num, uint8_t red, uint8_t green, uint8_t blue, uint8_t white, uint8_t brightness = 100, bool end_transfer = true)
	{
		for (uint32_t i = 0; i < num; i++)
		{
			this->set_pixel(red, green, blue, white, brightness);
		}
		if (end_transfer)
		{
			this->end_transfer();
		}
	}

	void set_all(uint8_t red, uint8_t green, uint8_t blue, uint8_t white, uint8_t brightness = 100)
	{
		for (uint32_t i = 0; i < this->num_leds; i++)
		{
			this->set_pixel(red, green, blue, white, brightness);
		}
		this->end_transfer();
	}

	void end_transfer()
	{
		this->SPI_peripheral.transfer(0);
		delay(1);
	}

private:
	inline void one()
	{
		this->SPI_peripheral.transfer(0xF8);
	}

	inline void zero()
	{
		this->SPI_peripheral.transfer(0xC0);
	}

	uint32_t num_leds;
	SPIClass &SPI_peripheral;
};