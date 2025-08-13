#include "accel_handler.h"
#include "MC3419.h"

MC3419 mc3419; // Global instance of the accelerometer

AccelHandler::AccelHandler(int SDA, int SCL) {
    Wire.begin(SDA, SCL);

    if (!mc3419.begin(0x4C)) {
        Serial.println("Failed to initialize MC3419 chip!");
        while (1);
    }
    Serial.println("MC3419 initialized.");
}

Values AccelHandler::read() {
    MC3419_Data accData = mc3419.readAccel();
    return { accData.x_g, accData.y_g, accData.z_g };
}
