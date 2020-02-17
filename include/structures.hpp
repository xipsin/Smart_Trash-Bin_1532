#ifndef ___STRUCTURES_H___
#define ___STRUCTURES_H___

#include "Arduino.h"

enum userType {Master, CleanMaster, User, 
                Guest, Total, New, notStaff};

enum materialType {none, glass, metal, 
					plastic, plasticCup, unknown, error};

struct settings {
  uint8_t users_amount;
  uint8_t row_memorysize; //no need
  uint8_t startAddress;
  uint8_t GuestAddress;
  uint8_t TotalAddress;
  uint8_t ClearAddress;
  uint8_t MasterAddress;
};

struct user_node {
  uint8_t uid[7];       // = 0xFF;
  uint8_t uid_size;     // = 7;
  uint8_t plastic_pcs;    // = 0;
  uint8_t metal_pcs;      // = 0;
  uint8_t glass_pcs;      // = 0;
  uint8_t plast_cup_pcs;    // = 0;
};

#define setSize sizeof(settings)
#define uSize sizeof(user_node)
settings sett;



void print_material(materialType material);

#endif