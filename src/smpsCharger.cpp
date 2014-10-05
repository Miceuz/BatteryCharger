#include <Arduino.h>
#include "hardware.h"
#include "smpsCharger.h"
#include "Psu.h"
#include "LeadAcidCharger.h"

PID voltagePid = {8, 1.50, 0};
PID currentPid = {2.1, 0.05, 0}; 

#define OVERCURRENT_INHIBIT 6
#define OVERCURRENT_SENSE 5

Psu psu(DAC_OUT, ISENSE, VSENSE, &currentPid, &voltagePid);
LeadAcidCharger charger(&psu);

void setup() {
  Serial.begin(9600);
  Serial.println("Hello");
  analogReference(INTERNAL);
  
  pinMode(DAC_OUT, OUTPUT);
  pinMode(OVERCURRENT_SENSE, INPUT);
  pinMode(OVERCURRENT_INHIBIT, OUTPUT);
  
  digitalWrite(DAC_OUT, LOW);
  digitalWrite(OVERCURRENT_INHIBIT, LOW);
  
  while(HIGH == digitalRead(OVERCURRENT_SENSE)){
    digitalWrite(DAC_OUT, LOW);
    digitalWrite(OVERCURRENT_INHIBIT, HIGH);
    Serial.println("Overcurrent situation!");
  }
  digitalWrite(OVERCURRENT_INHIBIT, LOW);
  
  charger.setBulkCurrent(10000);
  charger.startPrechargeState();
} 

void loop()  { 
  if(HIGH == digitalRead(OVERCURRENT_SENSE)){
    Serial.println("Overcurrent situation!");
    while(1);
  }

  charger.run();
}

