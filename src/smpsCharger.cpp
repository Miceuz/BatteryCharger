#include <Arduino.h>
#include "hardware.h"
#include "smpsCharger.h"
#include "Psu.h"

#define VOLTAGE_CTRL_INTERVAL 100

#define TOPPING_VOLTAGE 14400
#define FLOAT_VOLTAGE 13650

int avgCurrent = 0;
unsigned long absorptionStart = 0;
unsigned long lastVoltageControlTimestamp = 0;

Psu psu(DAC_OUT, ISENSE, VSENSE, {1,1,1}, {1,1,1});

byte isVoltageControlTime() {
  if(millis() - lastVoltageControlTimestamp >= VOLTAGE_CTRL_INTERVAL){
    lastVoltageControlTimestamp = millis();
    return 1;
  }
  return 0;
}

void setup()  { 
  Serial.begin(9600);
  analogReference(INTERNAL);
  pinMode(DAC_OUT, OUTPUT);
  digitalWrite(DAC_OUT, 0);

  unsigned int voltage = map(analogRead(VSENSE), 0, 1023, 0, MAX_VOLTAGE);//millivolts
  Serial.print("Voltage ");
  Serial.println(voltage);

  while(voltage <= 100) {
    Serial.println("Connect the battery");
    delay(1000);
    voltage = map(analogRead(VSENSE), 0, 1023, 0, MAX_VOLTAGE);//millivolts
  }  
  psu.setConstantCurrent(10000);
} 


void loop()  { 

  avgCurrent = (float)psu.getCurrent() * 0.005 + (float)avgCurrent*0.995;
  
  psu.servo();
  
  if(MODE_CV == psu.getMode() && isVoltageControlTime()) {
    Serial.print("CV, ");
    if(psu.getVoltage() >= TOPPING_VOLTAGE && psu.getCurrent() < 300) {
      psu.setConstantVoltage(FLOAT_VOLTAGE);
    } else {
      if(millis() - absorptionStart > 3000){
        if(psu.getCurrent() < 100) {
          Serial.println("Connect the battery");
          while(1);
        }
      }
    }
    
    
 } else if(MODE_CC == psu.getMode() && isVoltageControlTime()) {
    Serial.print("CC, ");
    
    if(psu.getVoltage() > TOPPING_VOLTAGE + 100) {
      psu.setConstantVoltage(TOPPING_VOLTAGE);
      absorptionStart = millis();
    }
  }  
  Serial.print(psu.getVoltage());
  Serial.print(", ");
  Serial.print(psu.getCurrent());
  Serial.print(", ");
  Serial.print(avgCurrent);
  Serial.println();

}

