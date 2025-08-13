#ifndef SETTINGS_H
#define SETTINGS_H

bool MIXING_ENABLED = false; //enables mixing on ch1 and 2 for drive. Most brushed ESCs alrady do this
bool FLIPPED_CORRECTION_ENABLED = true; //when upside down, your driving is corrected so controls are the same. Weapon will spin opposite way if BIDIRECTIONAL_WEAPON is set to true

bool BIDIRECTIONAL_WEAPON = false; //set whether your spinning weapon (ch3) is bidirectional or not. (By default, brushless ESCs are not bidirectional)
//*****NOTE: If you change your ESC settings to 'BIDIRECTIONAL', make sure the center is set to 1500! Set the MIN and MAX to 1000 and 2000


//REVERSE: 
bool INVERSE_CH1 = false; //if robot is turning the wrong way, invert this one
bool INVERSE_CH2 = false; //if driving forwards/backwards is mixed up, invert this one
bool INVERSE_CH3 = false; //if weapon is spinning wrong way (ONLY for bidirectional weapons)
//If your weapon is spinning the wrong way and is NOT bidirectional, do either of the following:
//  1) you can swap any 2 of the 3 wires connecting your weapon motor to the ESC
//  2) go to your ESC settings and change the weapon mode to "reversed"

//CHANNEL LIMITS: This limits the range of a channel. 1.0 is 100%, 0.55 is 55%, etc. Values MUST be between 0.0 and 1.0
float CH1_LIMIT = 1.0; //ch1 is usually for turning, and is often toned down for control
float CH2_LIMIT = 1.0;
float CH3_LIMIT = 1.0;

#endif