#include "Hardware.h"

#define DELAY_TIME_BEFORE_SERVO_DETACH      300
#define DELAY_TIME_BEFORE_BUTTON_READ       0
#define DELAY_TIME_BETWEEN_MEASURING        30



bool check_button(int pin = BTN_PIN)
{
  delay(DELAY_TIME_BEFORE_BUTTON_READ);
  return digitalRead(pin);
}


bool getRFID_UID(uint8_t * uid, int size)
{
    if ( ! mfrc522.PICC_ReadCardSerial())
        return 0;
    else
    {
        *uid = mfrc522.uid.uidByte;
        size = mfrc522.uid.size;
    }
    return 1;
}


int get_analogReadValue(int pin, int num)
{
  int sum = 0;
  for (int i=0; i<num; i++)
  {
    sum += analogRead(pin);
    delay(DELAY_TIME_BETWEEN_MEASURING);
  }
  return sum / num;
}


void calibrate_IR_sensors(int num = 10)
  {
      IR_SENSOR1_CALIBRATION_VALUE = get_analogReadValue(IR_SENSOR1_PIN, num);    //analogRead(IR_SENSOR1_PIN);
      IR_SENSOR2_CALIBRATION_VALUE = get_analogReadValue(IR_SENSOR2_PIN, num);    //analogRead(IR_SENSOR2_PIN);

      
    /// DEBUG_PRINT
    #ifdef _DEBUG_PRINT
        Serial.print('\n');
        Serial.print(IR_SENSOR1_CALIBRATION_VALUE);
        Serial.print('\t');
        Serial.println(IR_SENSOR2_CALIBRATION_VALUE);
        Serial.print('\n');
    #endif
    ///
  }


  float get_weightInGrams(int scans_number)
{
	ounces = scale.get_units(scans_number);
	grams = ounces * 0.035274;                                  // переводим вес из унций в граммы
	return grams;
}


void init_scale()
{
    scale.begin(DT, SCK);
    scale.set_scale();
    scale.tare();
    scale.set_scale(calibration_factor);   
}



void init_servos()
{
    platform_servo.attach(PLATFORM_SERVO_PIN);
    slide_servo.attach(SLIDE_SERVO_PIN);
    slide_servo.write(SLIDE_SERV_PLASTIC_ANGLE);
    platform_servo.write(PLATF_SERVO_CLOSED);
    delay(DELAY_TIME_BEFORE_SERVO_DETACH);///
    slide_servo.detach();
}


void init_RFID()
{
    SPI.begin();
    mfrc522.PCD_Init();
}


void init_Display()
{
    lcd.init();
    lcd.backlight();
    lcd.home();
}

/*
float scan_VolumeSurface()////////////////////////////////
{
	//2.3 - 2.8 cm between the bottom and plastic  thing
	//16.2 cm - length of plastic thing
	//3mm from plastic thing bottom to 1st hole bottom
	//3mm from plastic thing bottom to 2nd hole bottom
	// 12mm - holes diametr 
	return 0;
}
*/