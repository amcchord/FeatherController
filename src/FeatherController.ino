/*********************************************************************
This is an example for our Monochrome OLEDs based on SSD1306 drivers

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/category/63_98

This example is for a 128x32 size display using I2C to communicate
3 pins are required to interface (2 I2C and one reset)

Adafruit invests time and resources providing this open source code,
please support Adafruit and open-source hardware by purchasing
products from Adafruit!

Written by Limor Fried/Ladyada  for Adafruit Industries.
BSD license, check license.txt for more information
All text above, and the splash screen must be included in any redistribution
*********************************************************************/

#include <SPI.h>
#include <i2c_t3.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_MS_PWMServoDriver.h"
#include <PulsePosition.h>
#include <Servo.h>


#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

Adafruit_MotorShield AFMS = Adafruit_MotorShield();
Adafruit_DCMotor *myMotor = AFMS.getMotor(1);


//Trim for RC Inputs
const int rcMin = 1099;
const int rcMax = 1920;
int rcScale = rcMax - rcMin;
#define FAILSAFE false //Failsafe is disabled for now
#define DEADBAND 10 //If thrust values are within +/-10 of 0 assume they are 0
#define REJECTTHRESH 2200 //Rc values above this number are considered invalid
#define RCVR_PPM 20

//Define the PPM decoder object
PulsePositionInput myIn;

//Create some global variables to store the state of RC Reciver Channels
double rc1 = 0; // Turn
double rc2 = 0; // Thrust
double rc3 = 0; // Weapon Power
double rc4 = 0; // Weapon Directon
double rc5 = 0; // RainbowMode!
double rc6 = 0; // Failsafe (Not used yet)
int left = 0;
int right = 0;

#if (SSD1306_LCDHEIGHT != 32)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

void setup()   {

  pinMode(RCVR_PPM, INPUT_PULLDOWN);
//  digitalWrite(RCVR_PPM, 1);

  AFMS.begin();  // create with the default frequency 1.6KHz
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
  display.clearDisplay();
  display.display();
  myIn.begin(RCVR_PPM);
  display.setCursor(0,0);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.println("Booting... Standby");
  display.display();



//   myMotor->setSpeed(150);
//   myMotor->run(FORWARD);
// //  delay(5000);
//   myMotor->run(RELEASE);




}


void loop() {
  updateChannels();

  //Scale the raw RC input
  int thrust = round(((rc2 - rcMin)/rcScale) * 500) - 250; //Cast to -250-0-250
  int turn = round(((rc1 - rcMin)/rcScale) * 500) - 250; //Cast to -250-0-250
  int weapon = round(((rc3 - rcMin)/rcScale) * 180); //Cast to 0-250
  int direction = round(((rc4 - rcMin)/rcScale) * 10) - 5; //Cast to -5-0-5;
  int ledMode = round(((rc5 - rcMin)/rcScale) * 6);



  //Apply Deadband Correction
  if (thrust < DEADBAND && thrust > (DEADBAND * -1)){
  thrust = 0;
  }
  if (turn < DEADBAND && turn > (DEADBAND * -1)){
  turn = 0;
  }
  if (weapon < DEADBAND){
  weapon = 0;
  }



  //If we have a serial port attached we can debug our inputs.
  Serial.print("Thrust: ");
  Serial.print(thrust);
  Serial.print(" Turn: ");
  Serial.print(turn);
  Serial.print(" Weapon: ");
  Serial.print(weapon);
  Serial.print(" Direction: ");
  Serial.print(direction);
  Serial.print(" LedMode: ");
  Serial.println(ledMode);

  // text display tests
  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(1);
  display.print("Thrust: ");
  display.println(thrust);
  display.print("Turn:   ");
  display.println(turn);
  display.print("Left:  ");
  display.println(left);
  display.print("Right: ");
  display.println(right);
  display.setCursor(96,0);
  display.print("W: ");
  display.println(weapon);
  display.display();

  delay(10);
}


void updateChannels(){

  int num = myIn.available();
  if (num > 0) {

    int rc1t = myIn.read(1);
    int rc2t = myIn.read(2);
    int rc3t = myIn.read(3);
    int rc4t = myIn.read(4);
    int rc5t = myIn.read(5);
    int rc6t = myIn.read(6);

    //Don't register weird outliers!
    if (rc1t > 0 && rc1t < REJECTTHRESH){
      rc1 = rc1t;
    }
    if (rc2t > 0 && rc2t < REJECTTHRESH){
      rc2 = rc2t;
    }
    if (rc3t > 0 && rc3t < REJECTTHRESH){
      rc3 = rc3t;
    }
    if (rc4t > 0 && rc4t < REJECTTHRESH){
      rc4 = rc4t;
    }
    if (rc5t > 0 && rc5t < REJECTTHRESH){
      rc5 = rc5t;
    }
    if (rc6t > 0 && rc6t < REJECTTHRESH){
      rc6 = rc6t;
    }

    if (rc6 > 2000 && FAILSAFE){ //Will shutdown is reciever is programed correctly for failsafe
      rc1 = 0;
      rc2 = 0;
      rc3 = 0;
      rc4 = 0;
      rc5 = 0;
      rc6 = 0;
    }
  }
}


void testdrawchar(void) {
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);

  for (uint8_t i=0; i < 168; i++) {
    if (i == '\n') continue;
    display.write(i);
    if ((i > 0) && (i % 21 == 0))
      display.println();
  }
  display.display();
  delay(1);
}


void testscrolltext(void) {
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(10,0);
  display.clearDisplay();
  display.println("scroll");
  display.display();
  delay(1);

  display.startscrollright(0x00, 0x0F);
  delay(2000);
  display.stopscroll();
  delay(1000);
  display.startscrollleft(0x00, 0x0F);
  delay(2000);
  display.stopscroll();
  delay(1000);
  display.startscrolldiagright(0x00, 0x07);
  delay(2000);
  display.startscrolldiagleft(0x00, 0x07);
  delay(2000);
  display.stopscroll();
}
