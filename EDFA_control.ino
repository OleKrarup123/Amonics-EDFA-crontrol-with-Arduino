
#include <LiquidCrystal_I2C.h>
#include <assert.h>

#include <Wire.h>
//#include <LCD.h>

// Unanswered questions:
// 2.5 V ref voltage always reads 0. Why?
// How to connect the EN1# pin? It is seemingly directly coupled to ground always? 


// Issues
// Temperature reading stays around 22.93 C even if the whole device is 2heated up. Not sure about the reason.
// External power source not yet implemented.
// Possibly as a result of the issue above, the 2.5V reference voltage doesn't work. 


// Fixed issues
// Voltages between 0 and 4.73 (max V of the USB cable) can be sent to the EDFA board
// Rotary encoder works with set voltage and disable using switch. 


// Description of pins on EDFA circuit board
//  PIN 1   VCC:   Input voltage. Should always be 5V. Directly connected to the 5V PS by default 
//  PIN 2   VCC:   Same as above
//  PIN 3   EN1#:  PIN used to determine if EDFA output is active or diabled. If set to 0 (GND) power is enabled. If set to 5V, power is disabled.   
//  PIN 4   PD1 :  Read this pin to measure the output power of the laser. 3.0V = max. Max power consumption = 3.2W. 
//  PIN 5   Internal use
//  PIN 6   Internal use
//  PIN 7   GND
//  PIN 8   GND
//  PIN 9   Internal use
//  PIN 10  GND
//  PIN 11  SET1:   Pin used for setting the control voltage for the EDFA. Setting V=0 yields minimum, while V=5V gives max current to EDFA. 
//  PIN 12  CUR1:   Read this pin to figure out the current supplied to the EDFA. Do calibration 
//  PIN 13  Vref:   2.5 V reference output voltage. Allows one to compare the output from other pins to a known voltage 
//  PIN 14  TEC1:   Will measure 2.5V when the EDFA has a temperature of 25deg C. 1V change from this value is 0.9 deg C. 


// Define Arduino pins
//Input pins
const int pinEN1 = 4; // PIN  3 above 
const int pinSETx = 5; // PIN 11 above

// Output pins (analog)
const int aPD1      = 0; // PIN4 above
const int aCUR1     = 1; // PIN12 above
const int aVref     = 2; // PIN13 above
const int aTEC1     = 3; // PIN14 above

const int aSDA      = 4; // PIN SDA on LCD screen
const int aSCL      = 5; // PIN SCL on LCD screen


//LCD Stuff
LiquidCrystal_I2C lcd(0x27,20,4); // Set screen address

//Clears the specified line on the LCD
void clearline(int lineNumber){
   assert(lineNumber>=0);
   assert(lineNumber<=3);
   lcd.setCursor(0,lineNumber);
   lcd.print("                    ");
   lcd.setCursor(0,lineNumber);
  }

//Function for displaying the current power (0-100%) to the EDFA on the LDCD
void displayPower(int vpos){
  lcd.setCursor(0,3);
  String testString = String("Power:");

    if(vpos<10){
      testString=testString+String("  ")+vpos;
      }
    else if(10<=vpos<100){
      testString=testString+String(" ")+vpos;
      }
    else if (100==vpos){
      testString=testString+vpos;
      }
    
    testString=testString+"% ";
    
    lcd.print(testString); 
  
  }


//Define various parameters
//Define values one can measure from the EDFA
double valPD1;
double valCUR1;
double valVref;
double valTEC1;


// Rotary Encoder Pins
const int pinA=2; // Pin A for the rotary encoder. How a pulse arrives here relative to pin B determines if we step up or down 
const int pinB=3;
const int pinSW=8; // Detects button pushes on the rotary encoder. Used to enable/disable. 

int lastCount=0; // Variable to keep track of rotary encoder position


double SETvoltage; //Output Voltage for pinSETx (PIN11). Determines how much current is delivered to the EDFA

// Booleans to keep track of on/off status
bool disable=true;
bool disableflag=false;

void displayStatus(bool disable){
    clearline(0);
    clearline(1);
    
    lcd.setCursor(0,0);
    if(disable==true){
    lcd.print("Status: Disabled"); 
    lcd.setCursor(0,1);
    lcd.print("Push knob to enable");
    }
    
    if(disable==false){
    lcd.print("Status: **Enabled!**");
    lcd.setCursor(0,1);
    lcd.print("Push knob to disable"); 
    }
    
  }

float truncate(float val, byte dec) 
{
    float x = val * pow(10, dec);
    float y = round(x);
    float z = x - y;
    if ((int)z == 5)
    {
        y++;
    } else {}
    x = y / pow(10, dec);
    return x;
}

void displayTemp(float T){
  lcd.setCursor(13,3);
  lcd.print("T=");
  lcd.print(int( T*10)/10.0);
  }

  

// Integers to keep track of current to EDFA (0-100%)
volatile int virtualPosition=0;
volatile int virtualPositionBeforeDisable=0;
volatile int virtualPositionBeforeDisable2=0;


// Mapping function that works for doubles
double modifiedMap(double x, double in_min, double in_max, double out_min, double out_max)
{
 double temp = (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
 //temp = (int) (4*temp + .5);
 return (double) temp;///4;
}



// Function for detecting interrupts from the Rotary Encoder
void isr(){
  
  static unsigned long lastInterruptTime=0;
  unsigned long interruptTime = millis();
  if(interruptTime - lastInterruptTime>5){

      if(digitalRead(pinB)==LOW){
      virtualPosition --;
        }
      else{
        virtualPosition ++;
        }
        virtualPosition=min(100,max(0,virtualPosition));
        lastInterruptTime=interruptTime;
      }
  
  }

void setup() {
  // put your setup code here, to run once:

  // Open serial connection
  Serial.begin(9600);


  // Pin setup for Rotary Encoder
  pinMode(pinA,INPUT);
  pinMode(pinB,INPUT);
  pinMode(pinSW,INPUT_PULLUP);
  pinMode(pinSETx,OUTPUT);
  pinMode(pinEN1,OUTPUT);
  attachInterrupt(digitalPinToInterrupt(pinA),isr,LOW);

  //Initialize LCD screen
  lcd.init();
  lcd.init();
  lcd.clear();
  lcd.backlight();
  lcd.setCursor(1,0);
  lcd.print("***Amonics EDFA***");
  lcd.setCursor(6,2);
  lcd.print("Welcome!");
  
  delay(2000);
  lcd.clear();
  
  
  
  
  // Disable output and start the program

  disable=true;
  disableflag=false;
  displayStatus(disable);

   
  
}

void loop() {
//Read output voltages and return numbers in 0-1023 range
valPD1=analogRead(aPD1);  
valCUR1=analogRead(aCUR1);
valVref=analogRead(aVref);
valTEC1=modifiedMap(analogRead(aTEC1),0.0,1023.0,22.75,27.25); // Maps the voltage measurement of aTEC1 to temperature


delay(20);
//displayTemp(valTEC1);
 

if ((!digitalRead(pinSW))){

  
  //If enabled, disable and ramp down power to 0
  if (disable==false){
    displayStatus(disable);
    disable=!disable;
    displayStatus(disable);   

    //Ramp down power
    for(int i=virtualPosition; i>=0;i=i-10){
      SETvoltage =map(i,0,100,0,255);
      analogWrite(pinSETx,SETvoltage);
      delay(2);
      }   
    SETvoltage=0;  
    analogWrite(pinSETx,SETvoltage); // Make sure the output power is zero!
    analogWrite(pinEN1,HIGH); // Make sure the output power is zero!
    
    displayPower(virtualPosition);


    }
  // If disabled, ramp up power to current position  
  else if (disable==true){

    Serial.println("The output has just been enabled.");
    
    disable=!disable;
    disableflag=!disableflag;
    displayStatus(disable);
    //Ramp up to current position
    analogWrite(pinEN1,LOW); //Activate Output
    for(int i=0; i<=virtualPosition;i=i+10){
      SETvoltage =map(i,0,100,0,255);

      analogWrite(pinSETx,SETvoltage);
      
      delay(2);
      }
      
    // Make sure that the voltage is correct
    SETvoltage =map(virtualPosition,0,100,0,255);
    analogWrite(pinSETx,SETvoltage);

    Serial.println("The position is "+String(virtualPosition));
    Serial.println("The voltage is "+String(SETvoltage));
     
    displayPower(virtualPosition);

    Serial.println("Exiting the 'Output just enabled'-screen!");
    
    }
      
  while (!digitalRead(pinSW))
    delay(10);

}


if (disable==true && disableflag==false  ){//
    // If the EDFA is disabled, display this fact on the LCD.
   
    Serial.println("Just entered the 'disable=true' screen");
    //SETvoltage =map(virtualPosition,0,100,0,255);
   
    //analogWrite(pinSETx,SETvoltage);
    Serial.println(virtualPosition);
    Serial.println(SETvoltage);
    
    displayPower(virtualPosition);
    //displayStatus(disable);
    //Change status of disableflag to prevent this function from being called more than once in a row.
    disableflag=!disableflag;
    Serial.println("Just exited the 'disable=true' screen");
  }


if (disable==false && virtualPosition != lastCount){
  //If the enabled is active and a +/- tick has been measured, change voltage and update the LCD.

  Serial.println("Just entered the 'disable=false'-screen.");
  
  SETvoltage =map(virtualPosition,0,100,0,255);
  analogWrite(pinSETx,SETvoltage);

  Serial.println("The position is "+String(virtualPosition));
  Serial.println("The voltage is "+String(SETvoltage));
  
  lastCount=virtualPosition;

    displayPower(virtualPosition);
    Serial.println("Exiting the 'disable=false'-screen.");
  }

if (disable==true && virtualPosition != lastCount){
  //If the device is disabled and a +/- tick has been measured, update the display with the new power, but keep the EDFA disabled.

  Serial.println("Just entered the 'disabled' screen. The output is disabled, but the set output has changed");
  Serial.println("The position is "+String(virtualPosition));
  Serial.println("The voltage is "+String(SETvoltage));
  //SETvoltage =map(virtualPosition,0,100,0,255);
  //analogWrite(pinSETx,SETvoltage);
  lastCount=virtualPosition;
  displayPower(virtualPosition);
  }


}//End Loop
