#ifndef SETTINGS_H
#define SETTINGS_H

bool MIXING_ENABLED = false; //enables mixing on ch1 and 2
bool FLIPPED_CORRECTION_ENABLED = true; //when upside down, your driving is corrected so controls are the same. Weapon will spin opposite way if BIDIRECTIONAL_WEAPON is set to true

bool BIDIRECTIONAL_WEAPON = false; //set whether your spinning weapon (ch3) is bidirectional or not. It will spin the other way when robot

//REVERSE: 
bool INVERSE_CH1 = true; //if robot is turning the wrong way, invert this one
bool INVERSE_CH2 = true; //if forwards/backwards is mixed up, invert this one
bool INVERSE_CH3 = false; //if weapon is spinning wrong way

//CHANNEL LIMITS: This limits the range of a channel. 1.0 is 100%, 0.55 is 55%, etc. Values MUST be between 0.0 and 1.0
float CH1_LIMIT = 0.6; //ch1 is usually for turning, and is often toned down for control
float CH2_LIMIT = 1.0;
float CH3_LIMIT = 1.0;

#endif