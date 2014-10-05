#include "LeadAcidCharger.h"

LeadAcidCharger::LeadAcidCharger(Psu *_psu):
    psu(_psu)
{

}

void LeadAcidCharger::run() {
  if(isControlTime()){
    psu->servo();
    avgCurrent = (((int32_t) psu->getCurrent() - avgCurrent) >> 4) + avgCurrent;
    avgVoltage = (((int32_t) psu->getVoltage() - avgVoltage) >> 4) + avgVoltage;
    
    if(MODE_BULK == mode) {
      if(avgVoltage > absorptionVoltage + voltsPerDegreeCompensation()) {
        startAbsorptionStage();
      }
    } else if(MODE_ABSORPTION == mode) {
      if(millis() - stageStarted > 3000 && avgCurrent < bulkEndCurrent) {
        startFloatStage();
      }
    } else if(MODE_FLOAT == mode) {
    } else if(MODE_PRECHARGE == mode) {
      if(millis() - stageStarted > 3000 && avgVoltage > criticallyDischargedVoltage + voltsPerDegreeCompensation()) {
        startBulkStage();
      }
    }
    
    checkBatteryConnected();

    Serial.print(psu->getVoltage());
    Serial.print(", ");
    Serial.print(psu->getCurrent());
    Serial.print(", ");
    Serial.print(psu->getControllSignal());
    Serial.print(", ");
    Serial.print(avgVoltage);
    Serial.print(", ");
    Serial.print(avgCurrent);
    Serial.println();
  }
}

void LeadAcidCharger::startPrechargeState() {
  Serial.println("Starting PRECHARGE");
  mode = MODE_PRECHARGE;
  psu->setConstantCurrent(prechargeCurrent);
  stageStarted = millis();
}

void LeadAcidCharger::startBulkStage() {
  Serial.println("Starting BULK CHARGE");
  mode = MODE_BULK; 
  psu->setConstantCurrent(bulkCurrent);
  stageStarted = millis();
}

void LeadAcidCharger::startAbsorptionStage() {
  Serial.println("Starting ABSORPTION CHARGE");
  mode = MODE_ABSORPTION; 
  psu->setConstantVoltage(absorptionVoltage);
  stageStarted = millis();
}

void LeadAcidCharger::startFloatStage() {
  Serial.println("Starting FLOATING CHARGE");
  mode = MODE_FLOAT; 
  psu->setConstantVoltage(floatVoltage);
  stageStarted = millis();
}

void LeadAcidCharger::setBulkCurrent(uint16_t c) {
  bulkCurrent = c;
  prechargeCurrent = bulkCurrent / 10;
  bulkEndCurrent = bulkCurrent / 10;
}


void LeadAcidCharger::checkBatteryConnected() {
  if(millis() - stageStarted > 30000){
    if(psu->getCurrent() < 10) {
      psu->off();
      Serial.println("Connect the battery!");
      while(1);
    }
  }
}


int8_t LeadAcidCharger::getBatteryTemperature() {
  return 20;
}

//battery temp coeff -3.9 mV/C°/cell = -0.0234 V/C°
int16_t LeadAcidCharger::voltsPerDegreeCompensation() {
  return (int16_t)(getBatteryTemperature() - 20) * -234 / 10;
}

uint8_t LeadAcidCharger::isControlTime() {
  if(millis() - lastControlTimestamp >= VOLTAGE_CTRL_INTERVAL){
    lastControlTimestamp = millis();
    return 1;
  }
  return 0;
}

