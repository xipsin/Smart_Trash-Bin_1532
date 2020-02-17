#ifndef ___SMARTBIN_LOGIC_H___
#define ___SMARTBIN_LOGIC_H___

#include "Arduino.h"

#include "../include/structures.hpp"


#define ERROR_WEIGHT_HIGH_LIMIT       -25
#define PLASTIC_CUP_WEIGHT_LOW_LIMIT  -25
#define PLASTIC_CUP_WEIGHT_HIGH_LIMIT  30
#define METAL_WEIGHT_LOW_LIMIT         30
#define METAL_WEIGHT_HIGH_LIMIT        90
#define PLASTIC_WEIGHT_LOW_LIMIT       90
#define PLASTIC_WEIGHT_HIGH_LIMIT     400
#define GLASS_WEIGHT_LOW_LIMIT        700



#define DELAY_TIME_AFTER_SERVO_ATTACH      200
#define DELAY_TIME_OPENNING_PLATFORM       700
#define DELAY_TIME_CLOSING_PLATFORM        300

#define DELAY_TIME_EXTERNAL_BEFORE_SERVO_DETACH   300

#define DELAY_TIME_EXTERNAL_SHOWING_WASTETYPE       500
#define DELAY_TIME_EXTERNAL_SHOWING_USERINFO        1000
#define DELAY_TIME_EXTERNAL_SHOWING_TOTALINFO        1000



#define DELAY_TIME_AFTER_ANALIZ_ERROR 1000
#define DELAY_TIME_AFTER_ANALIZ_UNKNOWN 1000



userType checkServiceUser(uint8_t * uid, uint8_t size);
int getUserAddress();
materialType check_materialType();
int exec_fandomat(int address);

#endif