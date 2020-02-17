#include "Arduino.h"

#include "structures.hpp"
#include "Hardware.h"
#include "Internal_EEPROM_DataBase.h"

#include "SmartBin_logic.h"


//#define   _RESET_DATA_EVERY_RESTART
//#git define    _DEBUG_PRINT

#define HELLO_MESSAGE "Smart Trash Bin"

#define SERIAL_SPEED 9600

#define DELAY_TIME_NOTHING_HAPPENED     1000
#define DELAY_TIME_AFTER_SERVICE        800
#define DELAY_TIME_AFTER_WORKING        0


// 1) scan rfid_card
// 2) look rfid_card's UID in DataBase in EEPROM  // getUserAddress(uid, size)
// 3) if getUserAddress(..) returned address, go to the step 5
// 4) if getUserAddress(..) returned -1, call createNewUser(..)
// 5) get new waste, scan, sort
// 6) read the user from EEPROM and change its statistic



void setup()
{
    init_Display();
    lcd.print(HELLO_MESSAGE);

    Serial.begin(SERIAL_SPEED);
    
    init_RFID();
    init_servos();  
    init_scale();
    calibrate_IR_sensors();

    
    restoreSettingsFromEEPROM();
  

    /////  Use these to restore Arduino AppServiceData
    #ifdef _RESET_DATA_EVERY_RESTART
        restoreDefaultSettings();
        setClearUser();
        setMasterUser();
    #endif
}





void loop()
{
    // print a message on LCD1602
        lcd.clear();
        lcd.home();
        lcd.print(HELLO_MESSAGE);

    int address = -3;
    if (mfrc522.PICC_IsNewCardPresent()) {
        address = getUserAddress();
    }
    else if (check_button(BTN_PIN))
    {
        address = sett.GuestAddress;
        
        #ifdef _DEBUG_PRINT
            Serial.println(">>> Button was pressed!");///////////
        #endif
    }


    if (address > 0)
    {
        #ifdef _DEBUG_PRINT
            Serial.println(">>> Launching fandomat..");
        #endif

        exec_fandomat(address);
        delay(DELAY_TIME_AFTER_WORKING);
    }
    else if (address >= -2)
        delay(DELAY_TIME_AFTER_SERVICE);
    else
        delay(DELAY_TIME_NOTHING_HAPPENED);
}