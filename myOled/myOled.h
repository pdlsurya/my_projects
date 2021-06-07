#ifndef __MYOLED_H
#define __MYOLED_H

#define PLATFORM_AVR

#define HARDWARE_I2C

void initOled();

void initOled(uint8_t sda, uint8_t scl);

void clearDisplay();

void clearPart(uint8_t page, uint8_t start_pos, uint8_t end_pos);

void setCursorPos(uint8_t x, uint8_t y);

void displayBmp(const byte binArray[]);

void printChar(char C, uint8_t xpos, uint8_t ypos, uint8_t font_size);

void printString(const char* str, uint8_t xpos , uint8_t ypos, uint8_t font_size);

void setBattery(uint8_t percentage);

void setSignal(uint8_t level);

void setBar(uint8_t level, uint8_t page);

void display();

void setCursor(uint8_t x, uint8_t y);

void print7Seg_number(const char *str, uint8_t xpos, uint8_t ypos);
#endif