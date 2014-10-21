#include "LeadAcidCharger.h"

LeadAcidCharger::LeadAcidCharger(Psu *_psu, LiquidCrystal_I2C* _lcd):
    psu(_psu),
    lcd(_lcd)
{

}

void LeadAcidCharger::run() {
  if(isControlTime()){
    psu->servo();
    avgCurrent = (((int32_t) psu->getCurrent() - avgCurrent) >> 4) + avgCurrent;
    avgVoltage = (((int32_t) psu->getVoltage() - avgVoltage) >> 4) + avgVoltage;
    
    if(MODE_BULK == mode) {
        checkBatteryConnected();
        if(avgVoltage > absorptionVoltage + voltsPerDegreeCompensation()) {
            startAbsorptionStage();
        }
    } else if(MODE_ABSORPTION == mode) {
        checkBatteryConnected();
        if(millis() - stageStarted > 3000 && avgCurrent < bulkEndCurrent) {
            startFloatStage();
        }
    } else if(MODE_FLOAT == mode) {
    } else if(MODE_PRECHARGE == mode) {
        checkBatteryConnected();
        if(millis() - stageStarted > 3000 && avgVoltage > criticallyDischargedVoltage + voltsPerDegreeCompensation()) {
            startBulkStage();
        }
    }
    

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
    
    if(millis() - lcdOutTimestamp > 1000) {
        lcd->setCursor(0,1);
        lcd->print(avgVoltage / 1000);
        lcd->print(".");
        lcd->print(avgVoltage % 1000);
        lcd->print("V ");
        lcd->print(avgCurrent / 1000);
        lcd->print(".");
        lcd->print(avgCurrent % 1000);
        lcd->print("A");
        lcdOutTimestamp = millis();
    }

  }
}

void LeadAcidCharger::startPrechargeState() {
  Serial.println("Starting PRECHARGE");
  lcd->setCursor(0,0);
  lcd->print("PRECHARGE");

  mode = MODE_PRECHARGE;
  psu->setConstantCurrent(prechargeCurrent);
  stageStarted = millis();
}

void LeadAcidCharger::startBulkStage() {
  Serial.println("Starting BULK CHARGE");
  lcd->setCursor(0,0);
  lcd->print("BULK            ");
  mode = MODE_BULK; 
  psu->setConstantCurrent(bulkCurrent);
  stageStarted = millis();
}

void LeadAcidCharger::startAbsorptionStage() {
  Serial.println("Starting ABSORPTION CHARGE");
  lcd->setCursor(0,0);
  lcd->print("ABSORPTION      ");
  mode = MODE_ABSORPTION; 
  psu->setConstantVoltage(absorptionVoltage);
  stageStarted = millis();
}

void LeadAcidCharger::startFloatStage() {
  Serial.println("Starting FLOATING CHARGE");
  lcd->setCursor(0,0);
  lcd->print("FLOAT           ");
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
  if(millis() - stageStarted > 20000){
    if(psu->getCurrent() < 10) {
      psu->off();
      Serial.println("Connect the battery!");
      lcd->setCursor(0,0);
      lcd->print("NO BATTERY!     ");
      lcd->setCursor(0,1);
      lcd->print("CONNECT & RESET ");
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

