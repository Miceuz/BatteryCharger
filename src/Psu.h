#ifndef _PSU_H
#define _PSU_H

#include "Arduino.h"

#define MODE_CC 0
#define MODE_CV 1

#define MAX_VOLTAGE 16540
#define MAX_CURRENT 10000

typedef struct _PID {
    float P;
    float I;
    float D; } PID;

class Psu {
    public:
        Psu(uint8_t dacOutPin, uint8_t isensePin, uint8_t vsensePin, PID *ipid, PID *vpid);
        void setConstantVoltage(uint16_t voltage);
        void setConstantCurrent(uint16_t current);
        void servo();
        uint16_t getCurrent();
        uint16_t getVoltage();
        uint8_t getControllSignal();
        uint8_t getMode();

    private:
        void controll(int value);
        void pidIteration(float processVariable, PID *coeffs);
        void setVoltage(unsigned int newSetPoint);
        void setCurrent(unsigned int newSetPoint);

        uint8_t dacOutPin;
        uint8_t isensePin;
        uint8_t vsensePin; 
        uint8_t mode;

        unsigned int currentSetPoint;
        unsigned int voltageSetPoint;

        PID *ipid;
        PID *vpid;
        float errIntegral;
        float setPoint;
        int controlSignal;
        
        uint16_t current;
        uint16_t voltage;
};
#endif
