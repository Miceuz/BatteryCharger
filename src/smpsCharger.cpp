#include <Arduino.h>
#include "hardware.h"
#include "smpsCharger.h"
#include "Psu.h"

//milliseconds
#define VOLTAGE_CTRL_INTERVAL 100

uint16_t avgCurrent = 0;
uint16_t avgVoltage = 0;

unsigned long stageStarted = 0;
unsigned long lastControlTimestamp = 0;

uint8_t mode;

PID voltagePid = {7.1, 1.50, 0};
PID currentPid = {2.1, 0.05, 0}; 

Psu psu(DAC_OUT, ISENSE, VSENSE, &voltagePid, &currentPid);

//1.7V @ 25°C
uint16_t criticallyDischargedVoltage = 11700;

//for 100AH battery
uint16_t bulkCurrent = 10000;
uint16_t prechargeCurrent = bulkCurrent / 10;
uint16_t bulkEndCurrent = bulkCurrent / 10;

//2.4V per cell
uint16_t absorptionVoltage = 14400;
//2.275V per cell
uint16_t floatVoltage = 13650;

void setBulkCurrent(uint16_t c) {
  bulkCurrent = c;
  prechargeCurrent = bulkCurrent / 10;
  bulkEndCurrent = bulkCurrent / 10;
}

byte isControlTime() {
  if(millis() - lastControlTimestamp >= VOLTAGE_CTRL_INTERVAL){
    lastControlTimestamp = millis();
    return 1;
  }
  return 0;
}

int8_t getBatteryTemperature() {
  return 25;
}

//battery temp coeff -3.9 mV/C°/cell = -0.0234 V/C°
int16_t voltsPerDegreeCompensation() {
  return (int16_t)(getBatteryTemperature() - 20) * -234 / 10;
}

void startPrechargeState() {
  Serial.println("Starting PRECHARGE");
  mode = MODE_PRECHARGE;
  psu.setConstantCurrent(prechargeCurrent);
  stageStarted = millis();
}

void startBulkStage() {
  Serial.println("Starting BULK CHARGE");
  mode = MODE_BULK; 
  psu.setConstantCurrent(bulkCurrent);
  stageStarted = millis();
}

void startAbsorptionStage() {
  Serial.println("Starting ABSORPTION CHARGE");
  mode = MODE_ABSORPTION; 
  psu.setConstantVoltage(absorptionVoltage);
  stageStarted = millis();
}

void startFloatStage() {
  Serial.println("Starting FLOATING CHARGE");
  mode = MODE_FLOAT; 
  psu.setConstantVoltage(floatVoltage);
  stageStarted = millis();
}

void checkBatteryConnected() {
  if(millis() - stageStarted > 3000){
    if(psu.getCurrent() < 10) {
      Serial.println("Connect the battery!");
      while(1);
    }
  }
}

void setup() {
  Serial.begin(9600);

  analogReference(INTERNAL);
  pinMode(DAC_OUT, OUTPUT);
  digitalWrite(DAC_OUT, 0);

  setBulkCurrent(10000);
  startPrechargeState();
} 

void loop()  { 
  if(isControlTime()){
    psu.servo();
    avgCurrent = (float)psu.getCurrent() * 0.005 + (float)avgCurrent*0.995;
    avgVoltage = (float)psu.getVoltage() * 0.005 + (float)avgVoltage*0.995;
    
    if(MODE_BULK == mode) {
      if(avgVoltage > absorptionVoltage + voltsPerDegreeCompensation()) {
        startAbsorptionStage();
      }
    } else if(MODE_ABSORPTION == mode) {
      if(avgVoltage >= absorptionVoltage + voltsPerDegreeCompensation() && avgCurrent < bulkEndCurrent) {
        startFloatStage();
      }
    } else if(MODE_FLOAT == mode) {
    } else if(MODE_PRECHARGE == mode) {
      if(stageStarted > 3000 && psu.getVoltage() > criticallyDischargedVoltage + voltsPerDegreeCompensation()) {
        startBulkStage();
      }
    }
    
    checkBatteryConnected();

    Serial.print(psu.getVoltage());
    Serial.print(", ");
    Serial.print(psu.getCurrent());
    Serial.print(", ");
    Serial.print(avgCurrent);
    Serial.println();
  }
}

