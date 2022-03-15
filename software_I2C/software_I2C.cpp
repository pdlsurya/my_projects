#include <Arduino.h>
#include "software_I2C.h"


#define I2C_DELAY delayMicroseconds(5)
#define SET_SDA  pinMode(i2c_sda, INPUT)
#define CLEAR_SDA pinMode(i2c_sda, OUTPUT)
                  
#define SET_SCL  pinMode(i2c_scl, INPUT)
#define CLEAR_SCL pinMode(i2c_scl, OUTPUT)

//Global variable declarations
uint8_t i2c_scl;
uint8_t i2c_sda;
uint8_t ack;

uint8_t read_scl()
{
	return digitalRead(i2c_scl);

}

bool read_ack()
{
  SET_SDA;
  I2C_DELAY;
  SET_SCL;
  while(read_scl()==LOW); //Clock stretching
  I2C_DELAY;
  ack=digitalRead(i2c_sda);
  CLEAR_SCL;
  I2C_DELAY;
  return ack;
	
}


void i2c_init(uint8_t sda, uint8_t scl)
{
	i2c_sda=sda;
	i2c_scl=scl;
}



void send_bit(bool data_bit)
{
   if(data_bit)
   	 SET_SDA;
   else
   	CLEAR_SDA;

   I2C_DELAY;
   SET_SCL;
   while(read_scl()==0);
   I2C_DELAY;
   CLEAR_SCL;
   I2C_DELAY;
}

bool i2c_write(uint8_t data)
{
	for(uint8_t i=0; i<8;i++)
	{
		send_bit((data & (0x80>>i))>>(7-i));
	}

	return (read_ack()==0)?1:0;
}

void i2c_start(uint8_t address)
{
  //SDA and SCL are already in high state after the stop condition
	CLEAR_SDA;
	I2C_DELAY;
	CLEAR_SCL;

    while(i2c_write(address << 1)!=1);

}

void i2c_stop()
{
	CLEAR_SDA;
	SET_SCL;

	I2C_DELAY;

	SET_SDA;
  

}

