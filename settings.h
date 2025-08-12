#ifndef SETTINGS_H
#define SETTINGS_H

bool MIXING_ENABLED = false; //enables mixing on ch1 and 2
bool FLIPPED_CORRECTION_ENABLED = true; //when upside down, your driving is corrected so controls are the same. Weapon will spin opposite way if BIDIRECTIONAL_WEAPON is set to true

bool BIDIRECTIONAL_WEAPON = false; //set whether your spinning weapon (ch3) is bidirectional or not. It will spin the other way when robot

//reverse settings
bool INVERSE_CH1 = true;
bool INVERSE_CH2 = true;
bool INVERSE_CH3 = false;

#endif