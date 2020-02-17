#ifndef ___HARDWARE_H___
#define ___HARDWARE_H___



#include <Servo.h>
#include "HX711.h"
#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>


// 			Button
#define BTN_PIN 	2


// 			Servos
#define PLATFORM_SERVO_PIN      3
#define SLIDE_SERVO_PIN 	    5

#define PLATF_SERVO_CLOSED      170
#define PLATF_SERVO_OPENED      70

#define SLIDE_SERV_PLASTIC_ANGLE        175
#define SLIDE_SERV_METALL_ANGLE         5
#define SLIDE_SERV_GLASS_ANGLE          60
#define SLIDE_SERV_PLASTIC_CUP_ANGLE    135

Servo platform_servo;
Servo slide_servo;


// 			Tensoresistor (scale)
#define DT  				6
#define SCK 				7
#define NUM_OF_SCALE_MESUREMENTS 30

HX711 scale; 
float calibration_factor = 6.14;
float grams;
float ounces;


//          IR sensors
#define IR_SENSOR1_PIN A1
#define IR_SENSOR2_PIN A2
#define NUM_OF_IR_MESURES 10
int IR_SENSOR1_CALIBRATION_VALUE = 0;
int IR_SENSOR2_CALIBRATION_VALUE = 0;
#define IR_1_POROG 35
#define IR_2_POROG 35


// 			RFID reader
#define RST_PIN     		9
#define SS_PIN      		10

MFRC522 mfrc522(SS_PIN, RST_PIN);


// LCD 1602 Display
LiquidCrystal_I2C lcd(0x27,16,2);


bool check_button(int pin = BTN_PIN);
int get_analogReadValue(int pin, int num);
float get_weightInGrams(int scans_number);
bool getRFID_UID(uint8_t * uid, int size);
void calibrate_IR_sensors(int num = 10);
void init_scale();
void init_servos();
void init_RFID();
void init_Display();
#endif