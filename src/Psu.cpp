#include "Psu.h"
#include "Arduino.h"

Psu::Psu(uint8_t _dacOutPin, uint8_t _isensePin, uint8_t _vsensePin):
        dacOutPin(_dacOutPin),
        isensePin(_isensePin),
        vsensePin(_vsensePin)
{
    C_P = 2.1;
    C_I = 0.05;
    
    P = 7.1;
    I = 1.5;
}

void Psu::setConstantVoltage(uint16_t voltage){
    mode = MODE_CV;
    setVoltage(voltage);
}

void Psu::setConstantCurrent(uint16_t current) {
    mode = MODE_CC;
    setCurrent(current);
}

void Psu::servo(){
    current = map(analogRead(isensePin), 0, 1023, 0, MAX_CURRENT);//milliamps
    voltage = map(analogRead(vsensePin), 0, 1023, 0, MAX_VOLTAGE);//millivolts

    if(MODE_CC == mode) {
        servoCurrent(current);
    } else if(MODE_CV == mode) {
        servoVoltage(voltage);
    }
}

uint16_t Psu::getCurrent() {
    return current;
}

uint16_t Psu::getVoltage() {
    return voltage;
}

uint8_t Psu::getMode() {
    return mode;
}

/* Private methods */

void Psu::controll(int value) {
  if(value > 255) {
    value = 255;
  }
  if(value < 0) {
    value = 0;
  }
  analogWrite(dacOutPin, value);
}

void Psu::servoVoltage(unsigned int curVoltage) {
  float v = (float)curVoltage / (float) MAX_VOLTAGE;
  float error = vsp - v;
  float ctrl = error * P + errIntegral;
  int controlSignal = ctrl * 255;
  if(controlSignal < 255) {
    errIntegral += error * I;
  }
  //~ Serial.print(curVoltage);
  //~ Serial.print(", ");
  //~ Serial.print(controlSignal);
  controll(controlSignal);
}

void Psu::servoCurrent(unsigned int curCurrent) {
  float c = (float)curCurrent / (float) MAX_CURRENT;
  float error = csp - c;
  float ctrl = error * C_P + errIntegral;
  int controlSignal = ctrl * 255;
  if(controlSignal < 255) {
    errIntegral += error * C_I;
  }
  //~ Serial.print(curCurrent);
  //~ Serial.print(", ");
  //~ Serial.print(controlSignal);
  controll(controlSignal); 
}

void Psu::setVoltage(unsigned int newSetPoint) {
  voltageSetPoint = newSetPoint;
  vsp = (float) voltageSetPoint / (float) MAX_VOLTAGE;
}

void Psu::setCurrent(unsigned int newSetPoint) {
  currentSetPoint = newSetPoint;
  csp = (float) currentSetPoint / (float) MAX_CURRENT;
}
