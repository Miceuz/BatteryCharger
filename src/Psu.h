#ifndef _PSU_H
#define _PSU_H

#include "Arduino.h"

#define MODE_CC 0
#define MODE_CV 1

#define MAX_VOLTAGE 16540
#define MAX_CURRENT 10000

class Psu {
    public:
        Psu(uint8_t dacOutPin, uint8_t isensePin, uint8_t vsensePin);
        void setConstantVoltage(uint16_t voltage);
        void setConstantCurrent(uint16_t current);
        void servo();
        uint16_t getCurrent();
        uint16_t getVoltage();
        uint8_t getMode();

    private:
        void controll(int value);
        void servoVoltage(unsigned int curVoltage);
        void servoCurrent(unsigned int curCurrent);
        void setVoltage(unsigned int newSetPoint);
        void setCurrent(unsigned int newSetPoint);

        uint8_t dacOutPin;
        uint8_t isensePin;
        uint8_t vsensePin; 
        uint8_t mode;

        unsigned int currentSetPoint;
        float csp;
        float cerrIntegral;
        float C_P;
        float C_I;

        unsigned int voltageSetPoint;
        float vsp;
        float errIntegral;
        float P;
        float I;
        int controlSignal;
        
        uint16_t current;
        uint16_t voltage;
};
#endif
