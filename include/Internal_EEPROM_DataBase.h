#ifndef ___INTERNAL_EEPROM_DATABASE_H___
#define ___INTERNAL_EEPROM_DATABASE_H___

#include "structures.hpp"

#include <EEPROMex.h>
#include <EEPROMvar.h>


#define CLEAR_CODESIZE 4
#define CLEAR_CODE {0x32,0xA9,0x2E,0x1E,0,0,0}
#define MASTER_CODESIZE 4
#define MASTER_CODE {0x89,0x78,0x9B,0x6E,0,0,0}

#define MAX_USERS_AMOUNT 20
#define SETTINGS_EEPROM_ADDRESS 0
#define USERDATA_EEPROM_ADDRESS 72


void saveSettingsToEEPROM();
void restoreDefaultSettings();
void setClearUser();
void setMasterUser();
void clearGuestStatistic();
void clearTotalStatistic();
void clearUsersFromEEPROM(int rowNum = 0);
void restoreSettingsFromEEPROM();
int createNewUser(const uint8_t *uid, uint8_t size);
void print_UserInfo(user_node User, int address);
int getUserAddressByRFID_ID(uint8_t *uid, uint8_t size);

#endif