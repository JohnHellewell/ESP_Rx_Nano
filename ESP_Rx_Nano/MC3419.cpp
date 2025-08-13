#include "MC3419.h"

bool MC3419::begin(uint8_t i2c_addr) {
    _addr = i2c_addr;
    Wire.begin();

    // Check chip ID
    if (readReg8(MC3419_REG_CHIP_ID) != MC3419_CHIP_ID) {
        return false;
    }

    // Default config
    setMode(MODE_STANDBY);
    setRange(RANGE_8G);
    setSampleRate(SR_1000Hz);
    setMode(MODE_CWAKE);

    return true;
}

void MC3419::setMode(MC3419_Mode mode) {
    uint8_t val = readReg8(MC3419_REG_MODE) & 0xF0;
    writeReg8(MC3419_REG_MODE, val | (mode & 0x07));
}

void MC3419::setRange(MC3419_Range range) {
    _range = range;
    setMode(MODE_STANDBY);
    uint8_t val = readReg8(MC3419_REG_RANGE_C) & 0x0F;
    writeReg8(MC3419_REG_RANGE_C, val | ((range & 0x07) << 4));
    setMode(MODE_CWAKE);
}

void MC3419::setSampleRate(MC3419_SampleRate rate) {
    setMode(MODE_STANDBY);
    writeReg8(MC3419_REG_SR, rate);
    setMode(MODE_CWAKE);
}

MC3419_Data MC3419::readAccel() {
    uint8_t raw[6];
    MC3419_Data data;
    float range_g[5] = { 19.614f, 39.228f, 78.456f, 156.912f, 117.684f };
    float res = 32768.0f;

    Wire.beginTransmission(_addr);
    Wire.write(MC3419_REG_XOUT_LSB);
    Wire.endTransmission(false);
    Wire.requestFrom(_addr, (uint8_t)6);

    for (int i = 0; i < 6; i++) raw[i] = Wire.read();

    data.x = (int16_t)((raw[1] << 8) | raw[0]);
    data.y = (int16_t)((raw[3] << 8) | raw[2]);
    data.z = (int16_t)((raw[5] << 8) | raw[4]);

    data.x_g = data.x / res * range_g[_range];
    data.y_g = data.y / res * range_g[_range];
    data.z_g = data.z / res * range_g[_range];

    return data;
}

uint8_t MC3419::readReg8(uint8_t reg) {
    Wire.beginTransmission(_addr);
    Wire.write(reg);
    Wire.endTransmission(false);
    Wire.requestFrom(_addr, (uint8_t)1);
    return Wire.read();
}

void MC3419::writeReg8(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(_addr);
    Wire.write(reg);
    Wire.write(value);
    Wire.endTransmission();
}
