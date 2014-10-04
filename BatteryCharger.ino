#define ISENSE A7
#define VSENSE A6
#define DAC_OUT 3

#define MODE_CV 0
#define MODE_CC 1

#define VOLTAGE_CTRL_INTERVAL 100

#define TOPPING_VOLTAGE 14400
#define FLOAT_VOLTAGE 13650

#define MAX_VOLTAGE 16540
#define MAX_CURRENT 10000

byte mode = MODE_CC;

int avgCurrent = 0;
unsigned long absorptionStart = 0;

unsigned int voltageSetPoint = 9000;
float vsp = (float) voltageSetPoint / (float) MAX_VOLTAGE;
float errIntegral = 0;
float P = 7.1;
float I = 1.5;
int controlSignal = 0;

unsigned int currentSetPoint = 10000;
float csp = (float) currentSetPoint / (float) MAX_CURRENT;
float cerrIntegral = 0;
float C_P = 2.1;
float C_I = 0.05;


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
} 

unsigned long lastVoltageControlTimestamp = 0;
byte isVoltageControlTime() {
  if(millis() - lastVoltageControlTimestamp >= VOLTAGE_CTRL_INTERVAL){
    lastVoltageControlTimestamp = millis();
    return 1;
  }
  return 0;
}

void dacOutput(int value) {
  if(value > 255) {
    value = 255;
  }
  if(value < 0) {
    value = 0;
  }
  analogWrite(DAC_OUT, value);
}


void servoVoltage(unsigned int curVoltage) {
  float v = (float)curVoltage / (float) MAX_VOLTAGE;
  float error = vsp - v;
  float ctrl = error * P + errIntegral;
  int controlSignal = ctrl * 255;
  if(controlSignal < 255) {
    errIntegral += error * I;
  }
  Serial.print(curVoltage);
  Serial.print(", ");
  Serial.print(controlSignal);
  dacOutput(controlSignal);
}


void servoCurrent(unsigned int curCurrent) {
  float c = (float)curCurrent / (float) MAX_CURRENT;
  float error = csp - c;
  float ctrl = error * C_P + errIntegral;
  int controlSignal = ctrl * 255;
  if(controlSignal < 255) {
    errIntegral += error * C_I;
  }
  Serial.print(curCurrent);
  Serial.print(", ");
  Serial.print(controlSignal);
  dacOutput(controlSignal); 
}

void setVoltage(unsigned int newSetPoint) {
  voltageSetPoint = newSetPoint;
  vsp = (float) voltageSetPoint / (float) MAX_VOLTAGE;
}

void setCurrent(unsigned int newSetPoint) {
  currentSetPoint = newSetPoint;
  csp = (float) currentSetPoint / (float) MAX_CURRENT;
}


void loop()  { 
  unsigned int current = map(analogRead(ISENSE), 0, 1023, 0, MAX_CURRENT);//milliamps
  unsigned int voltage = map(analogRead(VSENSE), 0, 1023, 0, MAX_VOLTAGE);//millivolts
  
  if(MODE_CV == mode && isVoltageControlTime()) {
    avgCurrent = (float)current * 0.005 + (float)avgCurrent*0.995;
    Serial.print("CV ");
    servoVoltage(voltage);
    Serial.print(", ");
    Serial.print(current);
    Serial.print(", ");
    Serial.print(avgCurrent);
    Serial.println();
    if(voltage >= TOPPING_VOLTAGE && current < 300) {
      setVoltage(FLOAT_VOLTAGE);
    } else {
      if(millis() - absorptionStart > 3000){
        if(current < 100) {
          Serial.println("Connect the battery");
          while(1);
        }
      }
    }
    
    
 } else if(MODE_CC == mode && isVoltageControlTime()) {
    Serial.print("CC ");
    servoCurrent(current);
    Serial.print(", ");
    Serial.print(voltage);
    Serial.println();
    
    if(voltage > TOPPING_VOLTAGE + 100) {
      setVoltage(TOPPING_VOLTAGE);
      mode = MODE_CV;
      absorptionStart = millis();
    }
  }  
}

