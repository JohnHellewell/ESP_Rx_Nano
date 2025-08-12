#ifndef ACCEL_HANDLER_H
#define ACCEL_HANDLER_H

#include <Arduino.h>
#include "MC3419.h"

struct Values {
    float x;
    float y;
    float z;
};

class AccelHandler {
public:
    AccelHandler(int SDA, int SCL);
    Values read();
};

#endif
