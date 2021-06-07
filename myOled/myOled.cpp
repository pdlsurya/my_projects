#include <Arduino.h>
#include "font.h"
#include "myOled.h"

#if defined (HARDWARE_I2C)
#include <Wire.h>
#elif defined (SOFTWARE_I2C)
#include "software_I2C.h"
#endif

#define OLED_ADDRESS 0x3C
#define DISP_ON 0xAF
#define DISP_OFF 0xAE
#define NORM_MODE 0xA6
#define REV_MODE 0xA7
#define PAGE_ADDRESSING_MODE 0x02
#define TYPE_CMD 0x80
#define TYPE_DATA 0x40
#define CMD_ROL 0xA1
#define CMD_SCAN_COM63 0xC8

uint8_t char_cnt;
uint8_t num_cnt;
//uint8_t battery[14] = {0xFF, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0xC3, 0x3C};
//uint8_t signal[9] = {0x80, 0x00, 0xC0, 0x00, 0xF0, 0x00, 0xFC, 0x00, 0xFF};
//uint8_t bar[128];
#if defined (PLATFORM_ESP8266)
uint8_t dispBuffer[8][128];
uint8_t disp_row;
uint8_t disp_column;

#elif defined (PLATFORM_AVR)
uint8_t pageBuffer[128];
uint8_t column;
#endif

void beginDataTransmission(uint8_t address)
{   
	#if defined(HARDWARE_I2C)
	Wire.beginTransmission(address);
	Wire.write(TYPE_DATA);

	#elif defined(SOFTWARE_I2C)
	i2c_start(address);
	i2c_write(TYPE_DATA);
	#endif
}

void endDataTransmission()
{   
	#if defined(HARDWARE_I2C)
	Wire.endTransmission();

	#elif defined(SOFTWARE_I2C)
	i2c_stop();
	#endif
}

void sendData(uint8_t Data)
{
	#if defined(HARDWARE_I2C)
	Wire.write(Data);

	#elif defined(SOFTWARE_I2C)
	i2c_write(Data);
	#endif
}

void sendCommand(uint8_t cmd)
{
    #if defined(HARDWARE_I2C)
	Wire.beginTransmission(OLED_ADDRESS);
	Wire.write(TYPE_CMD);
	Wire.write(cmd);
	Wire.endTransmission();

	#elif defined(SOFTWARE_I2C)
	i2c_start(OLED_ADDRESS);
	i2c_write(TYPE_CMD);
	i2c_write(cmd);
	i2c_stop();
	#endif
}

void setCursorPos(uint8_t x, uint8_t y)
{
	// the SH1106 display starts at x = 2! (there are two columns of off screen pixels)
	x += 2;
	sendCommand(0xB0 + (y >> 3));
	// set lower column address  (00H - 0FH) => need the upper half only - THIS IS THE x, 0->127
	sendCommand((x & 0x0F));
	// set higher column address (10H - 1FH) => 0x10 | (2 >> 4) = 10
	sendCommand(0x10 + (x >> 4));
}

#if defined (PLATFORM_ESP8266)

void setCursor(uint8_t x, uint8_t y)
{
	disp_column=x;
	disp_row=y/8;
}

void display()
{
	uint8_t page, segments, column;

	for (page = 0; page < 8; page++)
	{
		// move to the beginning of the next page
		setCursorPos(0, page * 8);
		for (segments = 0; segments < 8; segments++)
		{
			beginDataTransmission(OLED_ADDRESS);
			for (column = 0; column < 16; column++)
				sendData(dispBuffer[page][column+segments*16]);
			endDataTransmission();
	 }
 }
}
#endif


void setPageMode()
{
	sendCommand(0x20);
	sendCommand(PAGE_ADDRESSING_MODE);
}

void setChargePump()
{
	sendCommand(0x8d);
	sendCommand(0x14);
}

#if defined (PLATFORM_AVR) //clearPart and update_page is used for AVR platform which doesnot use a display buffer. 
void clearPart(uint8_t page, uint8_t start_pos, uint8_t end_pos)
{
	uint8_t segments, column;
	setCursorPos(start_pos, page * 8);
	for (segments = 0; segments < (end_pos - start_pos) / 16; segments++)
	{
		beginDataTransmission(OLED_ADDRESS);
		for (column = 0; column < 16; column++)
			sendData(0x00);
		endDataTransmission();
	}
}

void update_page(uint8_t page, uint8_t page_buff[])
{
	setCursorPos(0, page * 8);
	for (uint8_t segments = 0; segments < 8; segments++)
	{
		beginDataTransmission(OLED_ADDRESS);
		for (uint8_t column = 0; column < 16; column++)
			sendData(page_buff[column + segments * 16]);
		endDataTransmission();
	}
}

#endif

void clearDisplay()
{
	#if defined (PLATFORM_AVR)
	uint8_t page, segments, column;
	// clear the buffer so we can fill the screen with zeroes
	for (page = 0; page < 8; page++)
	{
		// move to the beginning of the next page
		setCursorPos(0, page * 8);
		for (segments = 0; segments < 8; segments++)
		{
			beginDataTransmission(OLED_ADDRESS);
			for (column = 0; column < 16; column++)
				sendData(0x00);
			endDataTransmission();
	 }
 }

 #elif defined (PLATFORM_ESP8266)
// memset(dispBuffer,0,sizeof(dispBuffer));
 for(uint8_t i=0;i<8;i++)
 {
 	for(uint8_t j=0;j<127;j++)
 		dispBuffer[i][j]=0;
 }

 #endif
}
#if defined(HARDWARE_I2C)
void initOled()
{
	Wire.begin();
	Wire.setClock(400000);
#elif defined(SOFTWARE_I2C)
void initOled(uint8_t sda, uint8_t scl)
  {
     i2c_init(sda, scl);
#endif   
	sendCommand(DISP_ON);
	sendCommand(NORM_MODE);
	sendCommand(CMD_ROL); //Rotate 90 degrees
	sendCommand(CMD_SCAN_COM63); //start scan from COM63 to COM0
	setPageMode();
	setChargePump();
	clearDisplay();
}

void printChar(char C, uint8_t xpos, uint8_t ypos, uint8_t font_size)
{
	char chr;
	#if defined (PLATFORM_AVR)
	beginDataTransmission(OLED_ADDRESS);
    #endif

	if (font_size == 6)
	{
		for (uint8_t i = 0; i < 6; i++)
		{
			chr = pgm_read_byte(&font6x8[((int)C - 32) * 6 + i]);
			#if defined(PLATFORM_AVR)
			//sendData(chr);
            pageBuffer[column++]=chr;
			#elif defined (PLATFORM_ESP8266)
			dispBuffer[disp_row][disp_column++]=chr;
			
			#endif

		}
		#if defined (PLATFORM_AVR)
		endDataTransmission();
		#endif
	}
	else if (font_size == 16)
	{
	 #if defined (PLATFORM_AVR)	
	 setCursorPos(xpos + (char_cnt) * 8, ypos);
	 beginDataTransmission(OLED_ADDRESS);

	 #elif defined (PLATFORM_ESP8266)
	 setCursor(xpos+(char_cnt)*8,ypos);
	 #endif

	for (uint8_t i = 0; i < 8; i++)
		{
			chr = pgm_read_byte(&font16x8[((int)C - 32) * 16 + i]);
            #if defined (PLATFORM_AVR)
			sendData(chr);

			#elif defined (PLATFORM_ESP8266)
			dispBuffer[disp_row][disp_column++]=chr;
           
            #endif

		}

		#if defined (PLATFORM_AVR)
		endDataTransmission();
		setCursorPos(xpos + (char_cnt) * 8, ypos + 8);
		beginDataTransmission(OLED_ADDRESS);

		#elif defined (PLATFORM_ESP8266)
		setCursor(xpos + (char_cnt) * 8, ypos + 8);
		#endif

		for (uint8_t i = 8; i < 16; i++)
		{
			chr = pgm_read_byte(&font16x8[((int)C - 32) * 16 + i]);

			#if defined (PLATFORM_AVR)
			 sendData(chr);
             
			#elif defined (PLATFORM_ESP8266)
			dispBuffer[disp_row][disp_column++]=chr;
          
            #endif

		}
		#if defined (PLATFORM_AVR)
		endDataTransmission();
		#endif
		char_cnt++;
	}
	else
		return;
}

void printString(const char* str, uint8_t xpos, uint8_t ypos, uint8_t font_size)
{
	uint8_t len = strlen(str);
	char_cnt = 0;
	#if defined (PLATFORM_AVR)
	setCursorPos(xpos, ypos);
	for(uint8_t i=0; i<128;i++)
		pageBuffer[i]=0;
    column=0;
	#elif defined (PLATFORM_ESP8266)
	setCursor(xpos,ypos);
	#endif
	for (uint8_t i = 0; i < len; i++)
		printChar(str[i], xpos, ypos, font_size);
	#if defined (PLATFORM_AVR)
	update_page(ypos/8,pageBuffer);
	#endif
	char_cnt = 0;
}
#if defined(PLATFORM_ESP8266)
void print7Seg_digit(char C, uint8_t xpos, uint8_t ypos)
{
	 char chr;
	

     for(uint8_t column=0;column<16;column++)
     {
     for(uint8_t page=0; page<3;page++)
     {
     	setCursor(xpos+column+(num_cnt*16),ypos+page*8);
     	chr = pgm_read_byte(&font_7SEG[((int)C - 48) * 48 + (page+column*3)]);
		dispBuffer[disp_row][disp_column++]=chr;
     }

       }

       num_cnt++;
  }


void print7Seg_number(const char *str, uint8_t xpos, uint8_t ypos)
{
   uint8_t len=strlen(str);
   num_cnt=0;
   setCursor(xpos, ypos);
   for(uint8_t i=0; i<len; i++)
   	print7Seg_digit(str[i], xpos, ypos);
   num_cnt=0;
}
#endif
void displayBmp(const byte binArray[])
{
	uint8_t line = 0, i;
	uint8_t chr;
	for (uint8_t y = 0; y < 64; y += 8)
	{
		#if defined (PLATFORM_AVR)
		setCursorPos(0, y);

		#elif defined (PLATFORM_ESP8266)
		setCursor(0,y);
		#endif

		for (uint8_t parts = 0; parts < 8; parts++)
		{
            #if defined (PLATFORM_AVR)
			beginDataTransmission(OLED_ADDRESS);
			#endif
			for (uint8_t column = 0; column < 16; column++)
			{
				chr=pgm_read_byte(&binArray[line * 128 + i]);
				#if defined (PLATFORM_AVR)
				sendData(chr);

				#elif defined (PLATFORM_ESP8266)
				dispBuffer[disp_row][disp_column++]=chr;

				#endif
				i++;
			}
			#if defined (PLATFORM_AVR)
			endDataTransmission();
			#endif
		}
		i = 0;
		line++;
	}
}
/*
void setBattery(uint8_t percentage)
{
	uint8_t temp;
	temp = map(percentage, 0, 100, 2, 12);
	for (uint8_t i = 2; i <= temp; i += 2)
		battery[i] = 0xFF;

	for (uint8_t i = temp; i < 11; i++)
		battery[i] = 0x81;
	if (percentage == 100)
		battery[12] = 0xFF;
	else
		battery[12] = 0xC3;
	#if defined(PLATFORM_AVR)
	setCursorPos(110, 0);
	beginDataTransmission(OLED_ADDRESS);

	#elif defined (PLATFORM_ESP8266)
	setCursor(110,0);

	#endif

	for (uint8_t i = 0; i < 14; i++)
	{
		#if defined (PLATFORM_AVR)
		sendData(battery[i]);
	    #elif defined (PLATFORM_ESP8266)
	      dispBuffer[disp_row][disp_column++]=battery[i];
	    #endif 
     }
	#if defined (PLATFORM_AVR)       
	endDataTransmission();
	#endif
}

void setSignal(uint8_t level)
{
	if (level == 0 || level < 20)
	{
		for (uint8_t i = 2; i <= 8; i += 2)
			signal[i] = 0x00;
	}
	else if (level >= 20 && level <= 50)
	{
		signal[2] = 0xC0;
		signal[4] = 0xF0;

		for (uint8_t i = 6; i <= 8; i += 2)
			signal[i] = 0x00;
	}
	else if (level > 50 && level <= 80)
	{
		signal[2] = 0xC0;
		signal[4] = 0xF0;
		signal[6] = 0xFC;
		signal[8] = 0x00;
	}
	else
	{
		signal[2] = 0xC0;
		signal[4] = 0xF0;
		signal[6] = 0xFC;
		signal[8] = 0xFF;
	}

	#if defined (PLATFORM_AVR)
	setCursorPos(0, 0);
	beginDataTransmission(OLED_ADDRESS);

	#elif defined (PLATFORM_ESP8266)
	setCursor(0,0);
	#endif

	for (uint8_t i = 0; i < 9; i++)
	{
     #if defined (PLATFORM_AVR)
		sendData(signal[i]);

	 #elif defined (PLATFORM_ESP8266)
	   dispBuffer[disp_row][disp_column++]=signal[i];
	 #endif
	 }

	 #if defined (PLATFORM_AVR)  	
	endDataTransmission();
	#endif
}

inline void barInit()
{
	bar[0] = 0x3C;
	bar[1] = 0xC3;
	bar[126] = 0xC3;
	bar[127] = 0x3C;
}

void setBar(uint8_t level, uint8_t page)
{
	barInit();
	uint8_t temp = level;
	if (temp > 125)
		temp = 0;
	for (uint8_t i = 2; i <= temp; i++)
		bar[i] = 0xBD;

	for (uint8_t i = temp + 2; i <= 125; i++)
		bar[i] = 0x81;

	#if defined (PLATFORM_AVR)
	update_page(page, bar);

	#elif defined (PLATFORM_ESP8266)
	setCursor(0, page * 8);
	for (uint8_t column = 0; column < 128; column++)
	    dispBuffer[disp_row][disp_column++]=bar[column];
	
	#endif	

}
*/