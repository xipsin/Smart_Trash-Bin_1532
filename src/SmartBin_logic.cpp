#include "Arduino.h"

#include "Hardware.h"
#include "structures.hpp"
#include "SmartBin_logic.h"
#include "Internal_EEPROM_DataBase.h"



userType checkServiceUser(uint8_t *uid, uint8_t size)
{
  userType UserType = notStaff;
  user_node User;

  EEPROM.readBlock(sett.ClearAddress, User);
  if (size == User.uid_size &&
      memcmp(uid, User.uid, size) == 0)
  {
    UserType = CleanMaster;
    print_UserInfo(User, sett.ClearAddress);
    clearGuestStatistic();
    clearTotalStatistic();
    clearUsersFromEEPROM();
    Serial.println(">>> Вся статистика была успешно удалена!");
  }
  else
  {
    EEPROM.readBlock(sett.MasterAddress, User);
    if (size == User.uid_size &&
        memcmp(uid, User.uid, size) == 0)
    {
      UserType = Master;
      print_UserInfo(User, sett.MasterAddress);
      EEPROM.readBlock(sett.GuestAddress, User);
      print_UserInfo(User, sett.GuestAddress);
      EEPROM.readBlock(sett.TotalAddress, User);
      print_UserInfo(User, sett.TotalAddress);
    }
  }
  return UserType;
}


int getUserAddress()
{
    int address = 0;
    uint8_t *uid;
    int size;
    userType userT = Guest;

    if (!getRFID_UID(uid, size))
    {
      address = sett.GuestAddress;
      Serial.println(">>> Cant read card");
      //userT = Guest
    }
    
    //mfrc522.PICC_DumpDetailsToSerial(&(mfrc522.uid)); // RFID Details
    userT = checkServiceUser(uid, size);
    switch (userT)
    {
      case Master:
        address = -2;
        break;
      case CleanMaster:
        address = -1;
        break;
      case notStaff:
      {
        address = getUserAddressByRFID_ID(uid, size);
        Serial.println(">>> Searching the user in eeprom");
        if (address == -1)
        {
          userT = New;
          address = createNewUser(uid, size);
          Serial.println(">>> Creating a new User");
        }
        if (address == -1)
        {
          userT = Guest;
          address = sett.GuestAddress;
          Serial.println(">>> Cancelled creating user. Use Guest account");
        }
        break;
      }
      default:
      {
        address = sett.GuestAddress;
        Serial.println(">>> SMTH went wrong. Default case");
        //userT = Guest;
      }
    }
  Serial.println(address);
  return address;
}



materialType check_materialType()
{
  materialType material = none;

  int val1 = get_analogReadValue(IR_SENSOR1_PIN, NUM_OF_IR_MESURES) - IR_SENSOR1_CALIBRATION_VALUE;
  int val2 = get_analogReadValue(IR_SENSOR2_PIN, NUM_OF_IR_MESURES) - IR_SENSOR2_CALIBRATION_VALUE;

  float weight = get_weightInGrams(NUM_OF_SCALE_MESUREMENTS);
  
  //// Debug print
  #ifdef _DEBUG_PRINT
    Serial.print(">>> Weight is ");
    Serial.println(weight);

    Serial.print("\n>>> ");
    Serial.print(val1);
    Serial.print('\t');
    Serial.println(val2);
    Serial.print('\n');
  #endif
  ////

  if (abs(val1) <= IR_1_POROG && abs(val2) <= IR_2_POROG && weight < PLASTIC_CUP_WEIGHT_HIGH_LIMIT)
  {
      material = none;
  }
  else
  {
      ////
      if (weight < ERROR_WEIGHT_HIGH_LIMIT)
        material = error;
      else if (weight <= PLASTIC_CUP_WEIGHT_HIGH_LIMIT)
        material = plasticCup;
      else if (weight > METAL_WEIGHT_LOW_LIMIT && 
                weight <= METAL_WEIGHT_HIGH_LIMIT)
        material = metal;
      else if (weight > PLASTIC_WEIGHT_LOW_LIMIT && 
                weight <= PLASTIC_WEIGHT_HIGH_LIMIT)
        material = plastic;
      else if (weight >= GLASS_WEIGHT_LOW_LIMIT)
        material = glass;
      else
        material = unknown;
  }

  return material;
} 




/// \toDo: change enum material to tare_type
int exec_fandomat(int address)
{
  lcd.clear();
  lcd.home();
  lcd.print("Launch..");

  user_node User;
  user_node Total;
  EEPROM.readBlock(address, User);
  EEPROM.readBlock(sett.TotalAddress, Total);
  
  lcd.setCursor(0,1);
  lcd.print("Analyzing the waste..");
  materialType material = check_materialType();
  
  lcd.clear();
  lcd.home();
  print_material(material);  
  Serial.println("");
  
  // position slide_servo to special container
  int angle = 0; 

  switch(material){
    case none:      //There is nothing in the fandomate!
    {
        Serial.println("There is nothing in fandomat!");
        lcd.setCursor(0,1);
        lcd.print("There is nothing!");
        return 0;
    }
    case error:
    {
      Serial.println("There is some problem with scale calibration!");
      Serial.println("I'll thow it to metal box");
      //Total.error_pcs++;
      angle = SLIDE_SERV_METALL_ANGLE;
      delay(DELAY_TIME_AFTER_ANALIZ_ERROR);
      break;
    }
    case unknown:
    {
      Serial.println("There is unknown waste.. I'll thow it to metal box");
      //Total.unknown_pcs++;
      angle = SLIDE_SERV_METALL_ANGLE;
      delay(DELAY_TIME_AFTER_ANALIZ_UNKNOWN);
      break;
    }
    case glass:
      {
        User.glass_pcs++;
		    Total.glass_pcs++;
        angle = SLIDE_SERV_GLASS_ANGLE;
        break;  
      }
    case metal:
      {
		    User.metal_pcs++;
		    Total.metal_pcs++;  
        angle = SLIDE_SERV_METALL_ANGLE;
        break;  
      }
    case plastic:
      {
        User.plastic_pcs++;
		    Total.plastic_pcs++;
        angle = SLIDE_SERV_PLASTIC_ANGLE;
        break;  
      }
	  case plasticCup:
      {
        User.plast_cup_pcs++;
		    Total.plast_cup_pcs++;
        angle = SLIDE_SERV_PLASTIC_CUP_ANGLE;
        break;  
      }
  }

  //Show User INFO while sorting the waste!
  delay(DELAY_TIME_EXTERNAL_SHOWING_WASTETYPE);
  print_UserInfo(User, address);


  // sort the waste!
  slide_servo.attach(SLIDE_SERVO_PIN);
  delay(DELAY_TIME_AFTER_SERVO_ATTACH);
  
  slide_servo.write(angle);
  
  platform_servo.write(PLATF_SERVO_OPENED);
  delay(DELAY_TIME_OPENNING_PLATFORM);

  platform_servo.write(PLATF_SERVO_CLOSED);
  delay(DELAY_TIME_CLOSING_PLATFORM);

  delay(DELAY_TIME_EXTERNAL_BEFORE_SERVO_DETACH);
  slide_servo.detach();


  //Show Total INFO !
  delay(DELAY_TIME_EXTERNAL_SHOWING_USERINFO);
  print_UserInfo(Total, sett.TotalAddress);


  //  CALiBRATION
  scale.tare();     //calibrate scale after servo mooving
  calibrate_IR_sensors();


  // Save changes in EEPROM
  EEPROM.writeBlock(address, User);
  EEPROM.writeBlock(sett.TotalAddress, Total);

  delay(DELAY_TIME_EXTERNAL_SHOWING_TOTALINFO);
  return 0;
}