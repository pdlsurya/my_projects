#ifndef SOFTWARE_I2C_H
#define SOFTWARE_I2C_H


void i2c_init(uint8_t sda, uint8_t scl);
void i2c_start(uint8_t address);
bool i2c_write(uint8_t data);
void i2c_stop();


#endif // SOFTWARE_I2C_H
