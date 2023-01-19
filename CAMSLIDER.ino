
#include <Wire.h>
#include <math.h>

#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"

#include <AccelStepper.h>
#include <MultiStepper.h>
#include "bitmap.h"

#define limitSwitch 10
#define stepSensor 11
#define PinSW 3
#define PinCLK 2  
#define PinDT 4

#define OLED_RESET - 1
#define SCREEN_ADDRESS 0x3C
SSD1306AsciiWire oled;

AccelStepper stepper1(1, 6, 5); // (Type:driver, STEP, DIR)
AccelStepper stepper2(1, 8, 7);

MultiStepper StepperControl;


long gotoposition[2];

volatile long XInPoint = 0;
volatile long YInPoint = 0;
volatile long XOutPoint = 0;
volatile long YOutPoint = 0;
volatile long totaldistance = 0;
int flag = 0;
int temp = 0;
int i, j;
unsigned long switch0=0;
unsigned long rotary0=0;
float setspeed = 200;
float motorspeed;
float timeinsec;
float timeinmins;
volatile boolean TurnDetected;
volatile boolean rotationdirection;

volatile boolean useSensor;
volatile int layerCount;

int currentStateCLK;
int lastStateCLK;

void Switch()  
{
 if(millis()-switch0>50)
 {
  flag=flag+1;
  Serial.println("flag");
  Serial.println(flag);
 }
 switch0=millis();
}


void Rotary()
{
    currentStateCLK = digitalRead(PinCLK);
    if (currentStateCLK != lastStateCLK && currentStateCLK == 1) {
        if (digitalRead(PinDT) != currentStateCLK) {
            rotationdirection = 1;
        } else {
            rotationdirection = 0;
        }
    }
    TurnDetected = true;
    lastStateCLK = currentStateCLK;
    delay(5);
}

void setup()
{

    Serial.begin(115200);

    stepper1.setMaxSpeed(6400);
    stepper1.setSpeed(6400);
    stepper2.setMaxSpeed(6400);
    stepper2.setSpeed(6400);

    pinMode(limitSwitch, INPUT_PULLUP);
    pinMode(stepSensor, INPUT_PULLUP);
    
    pinMode(PinSW, INPUT_PULLUP);
    pinMode(PinCLK, INPUT);
    pinMode(PinDT, INPUT);

    Wire.begin();
    Wire.setClock(400000L);
    oled.begin(& Adafruit128x64, SCREEN_ADDRESS, OLED_RESET);
    oled.setFont(Adafruit5x7);
    oled.set2X();
    oled.clear();

    // Create instances for MultiStepper - Adding the 2 steppers to the StepperControl instance for multi control
    StepperControl.addStepper(stepper1);
    StepperControl.addStepper(stepper2);

    lastStateCLK = digitalRead(PinCLK);
    useSensor = 1;
    layerCount = 0;

    attachInterrupt(digitalPinToInterrupt(PinSW), Switch, RISING); // SW connected to D2
    attachInterrupt(digitalPinToInterrupt(PinCLK), Rotary, RISING); // CLK Connected to D3

    oled.println("Welcome");
    delay(2000);
    oled.clear();

    Serial.println("Homing");
    Home();

}

void Home()
{
    stepper1.setMaxSpeed(6400);
    stepper1.setSpeed(3200);
    stepper2.setMaxSpeed(6400);
    stepper2.setSpeed(3200);
    if (digitalRead(limitSwitch) == 1) {
        oled.println("Homing!");
    }

    while (digitalRead(limitSwitch) == 1) {
        stepper1.setSpeed(-3200);
        stepper1.runSpeed();

    }
    delay(20);
    stepper1.setCurrentPosition(0);
    stepper1.moveTo(200);
    while (stepper1.distanceToGo() != 0) {
        stepper1.setSpeed(3200);
        stepper1.runSpeed();
    }
    stepper1.setCurrentPosition(0);
    oled.clear();
}

void SetSpeed()
{
    oled.clear();
    while (flag == 10) {
        if (TurnDetected) {
            TurnDetected = false;  // do NOT repeat IF loop until new rotation detected
            if (rotationdirection) {
                setspeed = setspeed + 30;
            }
            if (!rotationdirection) {
                setspeed = setspeed - 30;
                if (setspeed < 0) {
                    setspeed = 0;
                }
            }

            oled.clear();
            oled.println("Speed");
            motorspeed = setspeed / 80;
            oled.print(motorspeed);
            oled.println(" mm/s");
            totaldistance = XOutPoint - XInPoint;
            if (totaldistance < 0) {
                totaldistance = totaldistance * (-1);
            }
            else {

            }
            timeinsec = (totaldistance / setspeed);
            timeinmins = timeinsec / 60;
            oled.println("Time");
            if (timeinmins > 1) {
                oled.print(timeinmins);
                oled.print(" min");
            }
            else {
                oled.print(timeinsec);
                oled.print(" sec");
            }

        }
        oled.clear();
        oled.println("Speed");
        motorspeed = setspeed / 80;
        oled.print(motorspeed);
        oled.print(" mm/s");
        totaldistance = XOutPoint - XInPoint;
        if (totaldistance < 0) {
            totaldistance = totaldistance * (-1);
        }
        else {

        }
        timeinsec = (totaldistance / setspeed);
        timeinmins = timeinsec / 60;
        oled.println("Time");
        if (timeinmins > 1) {
            oled.print(timeinmins);
            oled.print(" min");
        }
        else {
            oled.print(timeinsec);
            oled.print(" sec");
        }

    }

}

void stepperposition(int n)
{
    stepper1.setMaxSpeed(6400);
    stepper1.setSpeed(3200);
    stepper2.setMaxSpeed(6400);
    stepper2.setSpeed(3200);
    if (TurnDetected) {
        TurnDetected = false;  // do NOT repeat IF loop until new rotation detected
        if (n == 1) {
            if (!rotationdirection) {
                if (stepper1.currentPosition() - 500 > 0) {
                    stepper1.move(-500);
                    while (stepper1.distanceToGo() != 0) {
                        stepper1.setSpeed(-3000);
                        stepper1.runSpeed();
                    }
                }
                else {
                    while (stepper1.currentPosition() != 0) {
                        stepper1.setSpeed(-3000);
                        stepper1.runSpeed();
                    }
                }
            }

            if (rotationdirection) {
                if (stepper1.currentPosition() + 500 < 61000) {
                    stepper1.move(500);
                    while (stepper1.distanceToGo() != 0) {
                        stepper1.setSpeed(3200);
                        stepper1.runSpeed();
                    }
                }
                else {
                    while (stepper1.currentPosition() != 61000) {
                        stepper1.setSpeed(3000);
                        stepper1.runSpeed();

                    }
                }
            }
        }
        if (n == 2) {
            if (rotationdirection) {
                stepper2.move(-100);
                while (stepper2.distanceToGo() != 0) {
                    stepper2.setSpeed(-3000);
                    stepper2.runSpeed();
                }
            }
            if (!rotationdirection) {
                stepper2.move(100);
                while (stepper2.distanceToGo() != 0) {
                    stepper2.setSpeed(3000);
                    stepper2.runSpeed();
                }
            }
        }
    }
}


void loop()
{

    //Begin Setup
    if (flag == 0) {
        oled.clear();
        oled.println("Setup?");
        setspeed = 200;
    }

    //SetXin
    if (flag == 1) {
        oled.clear();
        oled.println("Set X In");

        while (flag == 1) {
            stepperposition(1);
        }
        XInPoint = stepper1.currentPosition();
    }
    //SetYin
    if (flag == 2) {
        oled.clear();
        oled.println("Set Y In");

        while (flag == 2) {
            stepperposition(2);
        }
        stepper2.setCurrentPosition(0);
        YInPoint = stepper2.currentPosition();
    }
    //SetXout
    if (flag == 3) {
        oled.clear();
        oled.println("Set X Out");

        while (flag == 3) {
            stepperposition(1);
            Serial.println(stepper1.currentPosition());
        }
        XOutPoint = stepper1.currentPosition();

    }
    //SetYout
    if (flag == 4) {
        oled.clear();
        oled.println("Set Y Out");

        while (flag == 4) {
            stepperposition(2);
        }
        YOutPoint = stepper2.currentPosition();
        oled.clear();

        // Go to IN position
        gotoposition[0] = XInPoint;
        gotoposition[1] = YInPoint;
        oled.clear();
        oled.println("Preview");

        stepper1.setMaxSpeed(3000);
        StepperControl.moveTo(gotoposition);
        StepperControl.runSpeedToPosition();
    }

    if (flag == 5) {
        oled.clear();
        oled.println("Set Sensor");
    }

    if (flag == 6) {
        oled.clear();
        oled.println((useSensor == 1) ? "Yes" : "No");
        while (flag == 6) {
            //Serial.println(flag);
            if (TurnDetected) {
                TurnDetected = false;
                useSensor = !useSensor;
            }
            oled.clear();
            oled.println((useSensor == 1) ? "Yes" : "No");
        }
    }

    if((flag == 7) && useSensor == 0){
      Serial.print("go to 9");
      flag = 9;
    }

    if (flag == 7) {
        oled.clear();
        oled.println("Set Layers");
    }

    if (flag == 8) {
        oled.clear();
        oled.println("Set Layers");
        while (flag == 8) {
            if (TurnDetected) {
                TurnDetected = false;
                layerCount = (rotationdirection) ? layerCount + 1 : layerCount - 1;
                if(layerCount < 1){
                  layerCount = 1;
                }
            }
            oled.clear();
            oled.print("Layers: ");
            oled.print(layerCount);
        }
    }

    //Display Set Speed
    if (flag == 9) {
        oled.clear();
        oled.println("Set Speed");
    }
    //Change Speed
    if (flag == 10) {
        oled.clear();
        SetSpeed();
    }
    //DisplayStart
    if (flag == 11) {
        oled.clear();
        oled.println("Start");
        delay(10);
    }
    //Start
    if (flag == 12) {
        oled.clear();
        oled.println((useSensor)? "Sensor Run" : "Normal Run");

        Serial.println("-----");
        Serial.println(XInPoint);
        Serial.println(XOutPoint);
        Serial.println(YInPoint);
        Serial.println(YOutPoint);
        Serial.println("-----");

        if(useSensor){

          int XStep = round(XOutPoint / layerCount);
          int YStep = round(YOutPoint / layerCount);
          
          int currentStep = 0;
          int StepSensorLastValue = digitalRead(stepSensor);
        
          while(currentStep < layerCount){
            int StepSensorCurrentValue = digitalRead(stepSensor);
            if(StepSensorCurrentValue != StepSensorLastValue){
              if( StepSensorCurrentValue == 1){
                  Serial.println("step");
                  currentStep = currentStep + 1;

                  oled.clear();
                  oled.print("Step ");
                  oled.println(currentStep);
                  oled.print(" of ");
                  oled.print(layerCount);
                  
                  gotoposition[0] = XStep * currentStep;
                  gotoposition[1] = YStep * currentStep;
                  stepper1.setMaxSpeed(setspeed);
                  StepperControl.moveTo(gotoposition);
                  StepperControl.runSpeedToPosition(); 
              }
              StepSensorLastValue = StepSensorCurrentValue;
            }
          }
        }else{
          gotoposition[0] = XOutPoint;
          gotoposition[1] = YOutPoint;
          stepper1.setMaxSpeed(setspeed);
          StepperControl.moveTo(gotoposition);
          StepperControl.runSpeedToPosition();          
        }

        flag = flag + 1;
    }
    //Slide Finish
    if (flag == 13) {
        oled.clear();
        oled.println("Finish");
        delay(10);

    }
    //Return to start
    if (flag == 14) {
        oled.clear();
        Home();
        flag = 0;
    }
}
