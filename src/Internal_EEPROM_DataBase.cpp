#include "structures.hpp"
#include "Internal_EEPROM_DataBase.h"
#include "Hardware.h"

void saveSettingsToEEPROM()
{
  EEPROM.writeBlock(SETTINGS_EEPROM_ADDRESS, sett);
}


void restoreDefaultSettings()
{
  sett.users_amount   = 0;
  sett.row_memorysize = uSize; //No need
  sett.startAddress   = USERDATA_EEPROM_ADDRESS;
  sett.GuestAddress   = setSize + uSize;
  sett.TotalAddress   = setSize + 2 * uSize;
  sett.ClearAddress   = setSize + 3 * uSize;
  sett.MasterAddress  = setSize + 4 * uSize;
  saveSettingsToEEPROM();
}



void setClearUser()
{
  user_node User = {CLEAR_CODE, CLEAR_CODESIZE, 0, 0, 0, 0};
  EEPROM.writeBlock(sett.ClearAddress, User);
}

void setMasterUser()
{
  user_node User = {MASTER_CODE, MASTER_CODESIZE, 0, 0, 0, 0};
  EEPROM.writeBlock(sett.MasterAddress, User);
}

void clearGuestStatistic()
{
  user_node User = {{0, 0, 0, 0, 0, 0, 0}, 7, 0, 0, 0, 0};
  EEPROM.writeBlock(sett.GuestAddress, User);
}

void clearTotalStatistic()
{
  user_node User = {{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, 7, 0, 0, 0, 0};
  EEPROM.writeBlock(sett.TotalAddress, User);
}


void clearUsersFromEEPROM(int rowNum = 0)
{
  sett.users_amount = rowNum;
  saveSettingsToEEPROM();
}


void restoreSettingsFromEEPROM()
{
  EEPROM.readBlock(SETTINGS_EEPROM_ADDRESS, sett);
}


int createNewUser(const uint8_t *uid, uint8_t size)
{

  if (sett.users_amount >= MAX_USERS_AMOUNT)
  {
    Serial.println(">>> Can't add a new User! Out of memory!");
    return -1;
  }
  user_node new_user = {0};
  memcpy(new_user.uid, uid, size);
  new_user.uid_size = size;
  int address = sett.startAddress + sett.users_amount * sett.row_memorysize;
  sett.users_amount++;

  EEPROM.writeBlock(address, new_user);
  saveSettingsToEEPROM();
  Serial.println(">>> A new user was added!");
  return address;
}



void print_UserInfo(user_node User, int address)
{
  bool print_statistic = 1;
  Serial.println("\n\n\n");
  lcd.clear();
  lcd.home();

  if (address == sett.GuestAddress)
  {
        lcd.print("Guest account");
        Serial.println("Guest account");
  }
  else if (address == sett.TotalAddress)    // never happends
  {
        lcd.print("Total statistic");
        Serial.println("Total statistic");
  }
  else if (address == sett.ClearAddress)
  {
        lcd.print("CleanMaster Card");
        Serial.println("CleanMaster Card");
        print_statistic = 0;
  }
  else if (address == sett.MasterAddress)
  {
        lcd.print("SuperUser Card");
        Serial.println("SuperUser Card");
        print_statistic = 0;
  }
  else
  {
        for (int i = 0; i < 7; i++) {
          Serial.print(User.uid[i], HEX);
          Serial.print("  ");
          lcd.print(User.uid[i], HEX);          // Need to write 0A agains _A
          lcd.setCursor((i+1)*2, 0);
        }
        Serial.println("");
  }
  

  if (print_statistic)
  {
      
      Serial.print("Plastic: ");
      Serial.println(User.plastic_pcs);
      lcd.setCursor(0,1);
      lcd.print(User.plastic_pcs);

      Serial.print("Metal: ");
      Serial.println(User.metal_pcs);
      lcd.setCursor(4,1);
      lcd.print(User.metal_pcs);

      Serial.print("Glass: ");
      Serial.println(User.glass_pcs);
      lcd.setCursor(8,1);
      lcd.print(User.glass_pcs);
      
      Serial.print("Plastic cups: ");
      Serial.println(User.plast_cup_pcs);
      lcd.setCursor(12,1);
      lcd.print(User.plast_cup_pcs);

      Serial.println("");
  }
}




int getUserAddressByRFID_ID(uint8_t *uid, uint8_t size)
{
  int index = -1;
  int adr = sett.startAddress;
  user_node User;

  for (int i = 0; i < sett.users_amount; i++)
  {
    EEPROM.readBlock(adr, User);
    if (size == User.uid_size &&
        memcmp(uid, User.uid, size) == 0)
    {
      index = i;
      break;
    }

    adr += sett.row_memorysize;
  }
  return index != -1 ? adr : -1;
}