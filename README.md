# ESP_Rx_Nano
Code for the ESP Rx Nano board- Combat Robotics

This is the BETA version. Many deliverables are yet to be implemented.


Instalation Instructions:
Step 1: Get the Arduino IDE ready
- Download the Arduino IDE: https://www.arduino.cc/en/software/
- Go to Tools → Board → Boards Manager
- Search for "ESP32"
- Install "esp32" by Espressif Systems

Step 2: Get the code
- Go to https://github.com/JohnHellewell/ESP_Rx_Nano 
- Code → Download .zip
- Extract the files onto your computer
- Open the folder, and open the "ESP_Rx_Nano.ino" file using the Arduino IDE. 

Step 3: Plug in & install
- Plug in the ESP Rx Nano board to your computer with USB-C. 
(Note: if it repeatedly disconnects, press and hold the onboard "boot" button, and while holding, short press the "Reset" button.)
- On top, click the 'Select Board' or 'ESP Family Device': 'Select Other Board & Port'
- For board, select "Nologo ESP32C3 Super Mini", and click on the available USB port. (If multiple ports show up, unplug and plug the board to see which one it is)

Step 4: Adjust settings to your needs
- On top, all of the files should be available.
- Go to "secrets_example.h"
- Change the items in quotes to contain your desired name and password used for connecting to your robot. Keep the quotes. Ctrl + S
- Go into your files and rename this file to "secrets.h"
- Go into "settings.h", and change any values to reflect your robot. These can be changed at anytime in the future
- At the top, click the Upload Arrow to upload the code to the ESP32. It may take a minute
- Once it is done, go to Tools → Serial Monitor. 
- In the serial monitor, select "115200 Baud"
- Once the code is uploaded, the serial monitor should display the output text from the board. Flip it over to watch it display when it flips!

Step 5: Python. Note: if you alrady have python installed, open "controller.py" with your perferred IDE
- If you do not have Python, follow this tutorial for installing Python and PyCharm: https://www.youtube.com/watch?v=YagM_FuPLQU
- open up Terminal or Command Prompt, and run "pip install pygame matplotlib"
- Open the "controller.py" file in PyCharm. 
- To connect to your robot: 
  - power on your ESP Rx Nano
  - plug in your game controller to your laptop
  - On your laptop, go to available WiFi networks, and select the network you renamed earlier. Enter your password
  - Once connected, run the "controller.py" file
  - If everything works, a graph will pop on screen
  - Press the left trigger to disable safety mode, and enjoy driving! Drive controls are on the rightjoystick



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