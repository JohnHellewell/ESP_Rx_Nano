# ESP_Rx_Nano
Code for the ESP Rx Nano board- Combat Robotics

This is the BETA version. Many deliverables are yet to be implemented.

In order to run the controller.py program, make sure you have the right libraries installed:
```pip install pygame matplotlib```

Functionality & deliverables:
- Chip creates a local network for controlling device to join
- Can output PWM signals on the output pins to control ESCs and servos* (on ch1-3)
- Can correct driving controls when upside down for more intuitive driving (only comparitive to Z-axis for now)
- Failsafe is implemented. When signal cuts, robot stops moving
- chip can be reprogrammed wirelessly

Deliverables NOT yet implemented:
- Support for ch4
- Dynamic "upside down" control: user specifies which direction is "right side up" and "upside down"
- Drive inversion can be toggled with a button on controller
- Optional: extend servo signal range to [500, 2500] rather than the typical [1000, 2000] on user-specified pins