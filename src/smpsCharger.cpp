#include <Arduino.h>
#include "hardware.h"
#include "smpsCharger.h"
#include "Psu.h"
#include "LeadAcidCharger.h"
#include <Wire.h>
#include "LiquidCrystal_I2C.h"

PID voltagePid = {8, 1.50, 0};
PID currentPid = {2.1, 0.05, 0}; 

#define OVERCURRENT_INHIBIT 6
#define OVERCURRENT_SENSE 7

Psu psu(DAC_OUT, ISENSE, VSENSE, &currentPid, &voltagePid);
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address

LeadAcidCharger charger(&psu, &lcd);


void setup() {
  Serial.begin(9600);
  lcd.begin(16,2);   // initialize the lcd for 16 chars 2 lines, turn on backlight

  analogReference(INTERNAL);
  
  pinMode(DAC_OUT, OUTPUT);
  pinMode(OVERCURRENT_SENSE, INPUT);
  pinMode(OVERCURRENT_INHIBIT, OUTPUT);
  
  psu.off();
  digitalWrite(OVERCURRENT_INHIBIT, LOW);
  
  while(HIGH == digitalRead(OVERCURRENT_SENSE)){
    psu.off();
    digitalWrite(OVERCURRENT_INHIBIT, HIGH);
    Serial.println("Overcurrent situation!");
  }
  digitalWrite(OVERCURRENT_INHIBIT, LOW);

  lcd.backlight(); // finish with backlight on  

  charger.setBulkCurrent(10000);
  charger.startPrechargeState();
} 

void loop()  { 
  if(HIGH == digitalRead(OVERCURRENT_SENSE)){
    psu.off();
    Serial.println("Overcurrent situation!");
    while(1);
  }

  charger.run();

}

