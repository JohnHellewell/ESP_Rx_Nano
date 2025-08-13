#ifndef MC3419_H
#define MC3419_H

#include <Arduino.h>
#include <Wire.h>

#define MC3419_I2C_ADDR  0x4C  // Default I2C address
#define MC3419_CHIP_ID   0xA4  // Expected chip ID

// Registers
#define MC3419_REG_CHIP_ID   0x18
#define MC3419_REG_MODE      0x07
#define MC3419_REG_RANGE_C   0x20
#define MC3419_REG_SR        0x08
#define MC3419_REG_XOUT_LSB  0x0D

// Modes
enum MC3419_Mode {
    MODE_SLEEP   = 0b000,
    MODE_CWAKE   = 0b001,
    MODE_STANDBY = 0b011
};

// Ranges
enum MC3419_Range {
    RANGE_2G  = 0b000,
    RANGE_4G  = 0b001,
    RANGE_8G  = 0b010,
    RANGE_16G = 0b011,
    RANGE_12G = 0b100
};

// Sample rates
enum MC3419_SampleRate {
    SR_50Hz  = 0x11,
    SR_100Hz = 0x13,
    SR_250Hz = 0x15,
    SR_500Hz = 0x16,
    SR_1000Hz = 0x17
};

struct MC3419_Data {
    int16_t x;
    int16_t y;
    int16_t z;
    float x_g;
    float y_g;
    float z_g;
};

class MC3419 {
public:
    bool begin(uint8_t i2c_addr = MC3419_I2C_ADDR);
    void setMode(MC3419_Mode mode);
    void setRange(MC3419_Range range);
    void setSampleRate(MC3419_SampleRate rate);
    MC3419_Data readAccel();

private:
    uint8_t _addr;
    MC3419_Range _range;

    uint8_t readReg8(uint8_t reg);
    void writeReg8(uint8_t reg, uint8_t value);
};

#endif

