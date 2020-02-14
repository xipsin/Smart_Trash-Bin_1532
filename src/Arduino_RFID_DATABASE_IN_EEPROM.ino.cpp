#include "Arduino.h"

#include "LowPower.h"//////////////////////////////

#include <Servo.h>
#include "HX711.h"

#include <EEPROMex.h>
#include <EEPROMvar.h>

#include <SPI.h>
#include <MFRC522.h>

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

// 			button
#define BTN_PIN 			2
// 			servoes
#define PLATFORM_SERVO_PIN 	3
#define SLIDE_SERVO_PIN 	5
// 			tensoresistor
#define DT  				6                                                // Указываем номер вывода, к которому подключен вывод DT  датчика
#define SCK 				7
#define NUM_OF_MESUREMENTS 30
// 			rfid reader
#define RST_PIN     		9
#define SS_PIN      		10

MFRC522 mfrc522(SS_PIN, RST_PIN);
LiquidCrystal_I2C lcd(0x27,16,2);

HX711 scale; 
float calibration_factor = 6.14;                              // вводим калибровочный коэффициент
float grams;                                                  // задаём переменную для измерений в граммах
float ounces;


#define PLATF_SERVO_CLOSED 170
#define PLATF_SERVO_OPENED 70

#define SLIDE_SERV_PLASTIC_ANGLE 170
#define SLIDE_SERV_METALL_ANGLE 70
#define SLIDE_SERV_GLASS_ANGLE 0
#define SLIDE_SERV_PLASTIC_CUP_ANGLE 170

Servo platform_servo;
Servo slide_servo;


#define CLEAR_CODESIZE 4
#define CLEAR_CODE {0x32,0xA9,0x2E,0x1E,0,0,0}
#define MASTER_CODESIZE 7
#define MASTER_CODE {0x4,0x79,0x54,0xC2,0xE5,0x5A,0x80}

#define MAX_USERS_AMOUNT 20
#define SETTINGS_EEPROM_ADDRESS 0
#define USERDATA_EEPROM_ADDRESS 72

#define HELLO_MESSAGE "Smart Trash Bin"

#define IR_SENSOR1_PIN A1
#define IR_SENSOR2_PIN A2
int IR_SENSOR1_CALIBRATION_VALUE = 0;
int IR_SENSOR2_CALIBRATION_VALUE = 0;
#define IR_1_POROG 35
#define IR_2_POROG 35


#define DELAY_TIME 1000


enum materialType {none, glass, metal, 
					plastic, plasticCup};

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
  sett.MasterAddress   = setSize + 4 * uSize;
  saveSettingsToEEPROM();
}


void print_UserInfo(user_node User, int address)
{
  bool print_statistic = 1;
  Serial.println("\n\n\n");
  //Serial.println(address);

  lcd.clear();
  lcd.home();

  if (address == sett.GuestAddress)
  {
        lcd.print("Guest account");
        Serial.println("Guest account");
  }
  else if (address == sett.TotalAddress)
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
          lcd.print(User.uid[i], HEX);/////////////////////////////////////////////
          lcd.setCursor(i*2,0);//////////////////////////////////////////////////////
        }
        Serial.println("");

        //lcd.print(...);
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


int checkServiceUser(uint8_t *uid, uint8_t size)
{
  int ServiceCode = 0;
  user_node User;

  EEPROM.readBlock(sett.ClearAddress, User);
  if (size == User.uid_size &&
      memcmp(uid, User.uid, size) == 0)
  {
    print_UserInfo(User, sett.ClearAddress);
    clearGuestStatistic();
    clearTotalStatistic();
    clearUsersFromEEPROM();
    Serial.println(">>> Вся статистика была успешно удалена!");
    ServiceCode = 1;
  }
  else
  {
    EEPROM.readBlock(sett.MasterAddress, User);
    if (size == User.uid_size &&
        memcmp(uid, User.uid, size) == 0)
    {
      print_UserInfo(User, sett.MasterAddress);
      EEPROM.readBlock(sett.GuestAddress, User);
      print_UserInfo(User, sett.GuestAddress);
      EEPROM.readBlock(sett.TotalAddress, User);
      print_UserInfo(User, sett.TotalAddress);
      ServiceCode = 2;
    }
  }
  return ServiceCode;
}


int getUserAddress(uint8_t *uid, uint8_t size)
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

// 1) scan rfid_card
// 2) look rfid_card's UID in DataBase in EEPROM  // getUserAddress(uid, size)
// 3) if getUserAddress(..) returned address, go to the step 5
// 4) if getUserAddress(..) returned -1, call createNewUser(..)
// 5) get new waste, scan, sort
// 6) read the user from EEPROM and change its statistic


bool check_button(int pin = BTN_PIN)
{
  delay(30);
  return digitalRead(pin);
}


int get_analogReadValue(int pin, int num)
{
  int sum = 0;
  for (int i=0; i<num; i++)
  {
    sum += analogRead(pin);
    delay(30);
  }
  return sum / num;
}

void calibrate_IR_sensors()
  {
      IR_SENSOR1_CALIBRATION_VALUE = get_analogReadValue(IR_SENSOR1_PIN, 10);    //analogRead(IR_SENSOR1_PIN);
      IR_SENSOR2_CALIBRATION_VALUE = get_analogReadValue(IR_SENSOR2_PIN, 10);    //analogRead(IR_SENSOR2_PIN);
  }

void setup()
{
  lcd.init();                      // Инициализация дисплея  
  lcd.backlight();                 // Подключение подсветки
  lcd.setCursor(0,0);              // Установка курсора в начало первой строки
  lcd.print(HELLO_MESSAGE);       // Набор текста на первой строке
  //lcd.setCursor(0,1);              // Установка курсора в начало второй строки
  //lcd.print("ArduinoMaster");       // Набор текста на второй строке

  calibrate_IR_sensors();
  /// DEBUG_PRINT
  Serial.print('\n');
  Serial.print(IR_SENSOR1_CALIBRATION_VALUE);
  Serial.print('\t');
  Serial.println(IR_SENSOR2_CALIBRATION_VALUE);
  Serial.print('\n');
  ///

  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();
  
  platform_servo.attach(PLATFORM_SERVO_PIN);
  slide_servo.attach(SLIDE_SERVO_PIN);

  slide_servo.write(SLIDE_SERV_PLASTIC_ANGLE);
  platform_servo.write(PLATF_SERVO_CLOSED);
  delay(300);
  
  scale.begin(DT, SCK);
  scale.set_scale();
  scale.tare();
  scale.set_scale(calibration_factor);

  
  restoreSettingsFromEEPROM();
  /////  Use these to restore Arduino AppServiceData
  //restoreDefaultSettings();
  //setClearUser();
  //setMasterUser();
  
  //LowPower.idle(SLEEP_FOREVER, ADC_OFF, TIMER2_OFF, TIMER1_OFF, TIMER0_OFF,
  //              SPI_OFF, USART0_ON, TWI_OFF);
}



/// \toDo: change 0,1,2 cases to enum
int getRfidUserAddressInEEPROM()
{
  int address = 0;
  /*
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    address = sett.GuestAddress;
    Serial.println("Cant Find Card");
  }
  else*/ 
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    address = sett.GuestAddress;
    Serial.println(">>> Cant read card");
  }
  else
  {
    uint8_t *uid = mfrc522.uid.uidByte;
    int size = mfrc522.uid.size;
    //mfrc522.PICC_DumpDetailsToSerial(&(mfrc522.uid)); // DEBUG_INFO
    switch (checkServiceUser(uid, size))
    {
      case 2:
        address = -2;                     //... = sett.MasterAddress;
        //Serial.println("MASTERCARD");
        break;
      case 1:
        address = -1;                      //... = sett.ClearAddress;
        //Serial.println("ClearCard");
        break;
      case 0:
      {
        address = getUserAddress(uid, size);
        Serial.println(">>> Searching the user in eeprom");
        if (address == -1)
        {
          address = createNewUser(uid, size);
          Serial.println(">>> Creating a new User");
        }
        if (address == -1)
        {
          address = sett.GuestAddress;
          Serial.println(">>> Cancelled creating user. Use Guest account");
        }
        break;
      }
      default:
      {
        address = sett.GuestAddress;
        Serial.println(">>> SMTH went wrong. Default case");
      }
    }
  }
  Serial.println(address);
  return address;
}


float get_weightInGrams(int scans_number)
{
	//for (int i = 0; i < scans_number; i ++) {                             // усредняем показания, считав значения датчика 10 раз
	//	units = + scale.get_units(), scans_number;                          // суммируем показания 10 замеров
	//}
	//units = units / scans_number;                                         // усредняем показания, разделив сумму значений на 10
	ounces = scale.get_units(scans_number);
	grams = ounces * 0.035274;                                  // переводим вес из унций в граммы
	return grams;
}


float scan_VolumeSurface()////////////////////////////////
{
	//2.3 - 2.8 cm between the bottom and plastic  thing
	//16.2 cm - length of plastic thing
	//3mm from plastic thing bottom to 1st hole bottom
	//3mm from plastic thing bottom to 2nd hole bottom
	// 12mm - holes diametr 
	return 0;
}



materialType check_materialType()
{
  materialType material = none;

  int val1 = get_analogReadValue(IR_SENSOR1_PIN, 10) - IR_SENSOR1_CALIBRATION_VALUE;
  int val2 = get_analogReadValue(IR_SENSOR2_PIN, 10) - IR_SENSOR2_CALIBRATION_VALUE;

  //// Debug Print
  Serial.print("\n>>> ");
  Serial.print(val1);
  Serial.print('\t');
  Serial.println(val2);
  Serial.print('\n');
  ////

  if (abs(val1) <= IR_1_POROG && abs(val2) <= IR_2_POROG)
  {
      material = none;
  }
  else
  {
      float weight = get_weightInGrams( NUM_OF_MESUREMENTS);
      //// Debug print
      Serial.print(">>> Weight is ");
      Serial.println(weight);
      ////
      if (weight <= 25)
        material = plasticCup;
      else if (weight >= 25 and weight <= 90)
        material = metal;
      else if (weight >= 90 and weight <= 400)
        material = plastic;
      else if (weight >= 700)
        material = glass;
  }
  //float size = get_sizeInMM(10);
  //scan_VolumeSurface();

  return material;
} 

/*
materialType check_materialType() //////////////////////////////////////////////////////
{
  static int numCall;
  materialType mat;
  switch (numCall)
  {
    case 0:
      mat = none;
      break;
    case 1:
      mat = glass;
      break;
    case 2:
      mat = metal;
      break;
    case 3:
      mat = plastic;
      break;
    case 4:
      mat = plasticCup;
  } 
  numCall = (numCall+1) % 5;
  return mat;
}
*/



 void print_material(materialType material)
 {
   char str[15];
   switch(material){
    case none:      //There is nothing in the fandomate!
        sprintf(str, "empty");
        break;
    case glass:
        sprintf(str, "glass");
        break;  
    case metal:
        sprintf(str, "metal");
        break;  
    case plastic:
        sprintf(str, "plastic");
        break;  
	  case plasticCup:
        sprintf(str, "plastic cup");
        break;  
  }
  Serial.print(str);
  lcd.print(str);
 }



/// \toDo: change enum material to tare_type
/// \toDo calibrate scale
int exec_fandomat(int address)
{
  lcd.clear();
  lcd.home();
  user_node User;
  user_node Total;
  EEPROM.readBlock(address, User);
  EEPROM.readBlock(sett.TotalAddress, Total);
    
  //print_UserInfo();
  lcd.print("Launch..");

  materialType material = check_materialType();
  
  lcd.clear();
  lcd.home();
  print_material(material);  
  Serial.println("");
  //Serial.println(material);
  // print type of tare
  // position slide_servo to special container
  int angle = 0;
 //char message[8];
 

 switch(material){
    case none:      //There is nothing in the fandomate!
    {
        Serial.println("There is nothing in fandomat!");
        lcd.setCursor(0,1);
        lcd.print("There is nothing!");
        return 0;
    }
    case glass:
      {
        User.glass_pcs++;
		    Total.glass_pcs++;
        angle = SLIDE_SERV_GLASS_ANGLE;
        //message = "Glass";
        break;  
      }
    case metal:
      {
		    User.metal_pcs++;
		    Total.metal_pcs++;  
        angle = SLIDE_SERV_METALL_ANGLE;
        //message = "Metal";
        break;  
      }
    case plastic:
      {
        User.plastic_pcs++;
		    Total.plastic_pcs++;
        angle = SLIDE_SERV_PLASTIC_ANGLE;
        //message = "Plastic";
        break;  
      }
	  case plasticCup:
      {
        User.plast_cup_pcs++;
		    Total.plast_cup_pcs++;
        angle = SLIDE_SERV_PLASTIC_CUP_ANGLE;
        //message = "Plastic";
        break;  
      }
  }
  
  //lcd.print(message);
  delay(1500);
  print_UserInfo(User, address);
  delay(1500);
  print_UserInfo(Total, sett.TotalAddress);

  // sort the trash!
  slide_servo.write(angle);
  delay(200);
  platform_servo.write(PLATF_SERVO_OPENED);
  delay(500);
  platform_servo.write(PLATF_SERVO_CLOSED);
  delay(500);

  //  CALiBRATION
  scale.tare();     //calibrate scale after servo mooving
  calibrate_IR_sensors();

  // Save changes in EEPROM
  EEPROM.writeBlock(address, User);
  EEPROM.writeBlock(sett.TotalAddress, Total);

  return 0;
}



void loop()
{

  // print a message
    lcd.clear();
    lcd.home();
    lcd.print(HELLO_MESSAGE);

  int address = -1;
  if (mfrc522.PICC_IsNewCardPresent()) {
    address = getRfidUserAddressInEEPROM();
  }
  else if (check_button(BTN_PIN))
  {
    address = sett.GuestAddress;
    Serial.println(">>> Button was pressed!");///////////
  }

  if (address > 0)
  {
    //Display User UID or Name (Guest/Master/Clean)
    Serial.println(">>> Starting fandomat..");
    exec_fandomat(address);
    delay(DELAY_TIME);  //additional delay after exec
  }
  else if (address >= -2)
    delay(DELAY_TIME); //additional delay after serviceCard

  delay(DELAY_TIME);
  //LowPower.powerDown(SLEEP_250MS, ADC_OFF, BOD_OFF);  
}
