#include "structures.hpp"
#include "Hardware.h"


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
    case unknown:
        sprintf(str, "unknown");
        break;  
	  case error:
        sprintf(str, "error");
        break;  
  }
  Serial.print(str);
  lcd.print(str);
 }