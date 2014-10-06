#ifndef _LEAD_ACID_CHARGER_H
#define _LEAD_ACID_CHARGER_H

#include "Psu.h"
#include "Arduino.h"
#include "LiquidCrystal_I2C.h"

//milliseconds
#define VOLTAGE_CTRL_INTERVAL 100

#define MODE_PRECHARGE 0
#define MODE_BULK 1
#define MODE_ABSORPTION 2
#define MODE_FLOAT 3

class LeadAcidCharger {
    public:
        LeadAcidCharger(Psu*, LiquidCrystal_I2C*);
        void run(void);
        void setBulkCurrent(uint16_t);
        void startPrechargeState();
        void startBulkStage();
        void startAbsorptionStage();
        void startFloatStage();
        
    private:
        void checkBatteryConnected();
        int8_t getBatteryTemperature();
        int16_t voltsPerDegreeCompensation();
        uint8_t isControlTime();
        
        Psu *psu;
        LiquidCrystal_I2C *lcd;
        int32_t avgCurrent = 0;
        int32_t avgVoltage = 0;

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

        unsigned long stageStarted = 0;
        unsigned long lastControlTimestamp = 0;
        unsigned long lcdOutTimestamp = 0;

        uint8_t mode;

};

#endif
