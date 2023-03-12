/*
On DigiSpark, Pins PB3 and PB4 are required for USB comms and PB 5 makes for
a poor analog input as it resets the micro when pulled too low. (Yes, reset
can be disabled, but that means that one needs a high voltage programmer on
subsequent attempts to flash a new program.)
That leaves only PB2 as an analog input and a joystick with only one axis is
not very useful.
So, an ADS1115 4-channel I2C ADC has been attached to the DigiSpark to give
four analog joystick inputs. 
Do what you will with this; it was an exercise in getting 4 good analog
inputs from a DigiSpark using an I2C ADC.
Ken McMullan, 12-Mar-2023
*/

#include "DigiJoystick.h"
#include <Wire.h>

#define Addr 0x48 // with ADDR to Gnd, ADS1115 7-bit I2C address is 0x48

#define LED_PIN 1 // DigiSpark Model "A" convention

// jBuf = { x, y, xrot, yrot, zrot, slider, buttonLowByte, buttonHighByte }
#define byteX 0 // x
#define byteY 1 // y
#define byteZ 4 // zrot
#define byteW 5 // slider
#define byteB 6 // buttons

char jBuf[8]; // char jBuf[8] = { x, y, xrot, yrot, zrot, slider, buttonLowByte, buttonHighByte };
char aBuf[2]; // MSB:LSB as read from the I2C device
char tog = 0; // state of watchdog LED
char mux = 0; // cyclic ADC multiplexer

// select a multiplexer and start single-shot acquisition
void selADC(char mux)
{
  char confH = 0b11000011;        // Start single-shot conversion, AINP = AIN0 and AINN = gnd, +/- 4.096V
  // +/- 4.096V has been selected since we're referencing the USB voltage (0-5V)
  // and the potentiometers never go full swing

  confH &= 0b10001111;            // clear conf bits 14:12 (multiplexer)
  confH |= (mux | 4) << 4;        // set bit 2 and move mux into conf bits 14:12

  Wire.begin();                  // Initialise I2C communication as MASTER
  Wire.beginTransmission(Addr);  // Start I2C Transmission
  Wire.write(0x01);              // Select configuration register

  Wire.write(confH);             // Configuration high byte
  Wire.write(0b10000011);        // Conf low byte => single-shot mode, 128 SPS

  Wire.endTransmission();        // Stop I2C Transmission

} // selADC

// ascertain if the ADC is completed
bool isReadyADC()
{
  bool ready = false;
  Wire.begin();                  // Initialise I2C communication as MASTER
  Wire.beginTransmission(Addr);  // Start I2C Transmission
  Wire.write(0x01);              // Select configuration register

  Wire.requestFrom(Addr, 1);    // Request a bytes of data (don't care about low byte)
  
  if (Wire.available() == 1)
  {
    if (Wire.read() && 0b10000000) // read MS byte of 16 byte config register
    { ready = true; }              // config bit 15: 1=not currently performing a conversion
  }

  return ready;
} // isReadyADC

void setup()
{
  pinMode(LED_PIN, OUTPUT);
  selADC(0); // to kick off single-shot mode, etc.
  for (mux=0; mux<=5; mux++) { jBuf[mux] = 127; } // all joysticks in the middle by default, buttons off
}

void loop()
{

  if ( isReadyADC() )
  {
    Wire.beginTransmission(Addr); // Start I2C Transmission
    Wire.write(0x00);             // Select conversion register
    Wire.endTransmission();       // Stop I2C Transmission

//    Wire.requestFrom(Addr, 2);    // Request 2 bytes of data (16-bit ADC)
    Wire.requestFrom(Addr, 1);    // Request 1 byte of data (MS byte of 16-bit ADC)

    if (Wire.available() == 1)    // Read requested bytes of data: the ADV value, MSbyte first
    {
      aBuf[0] = Wire.read();      // MS byte - LSbyte is not used
//      aBuf[1] = Wire.read(); // LS byte is not used - reminder to self that it's a 16-bit ADC!

      if (mux == 0) { jBuf[byteW] = aBuf[0]; } // shuffle MSbytes into joystick buffer
      else if (mux == 1) { jBuf[byteX] = aBuf[0]; }
      else if (mux == 2) { jBuf[byteY] = aBuf[0]; }
      else if (mux == 3) { jBuf[byteZ] = aBuf[0]; }

      DigiJoystick.setValues(jBuf); // output the joystick buffer if it's been updated

    } // if available

    mux += 1;          // next ADC
    if (mux >= 4) {
      mux = 0;         // cycle ADC counter without overflow

      if (tog == 0) {tog = 1;} else {tog = 0;} // watchdog LED toggles when 4th ADC completes
      digitalWrite(LED_PIN, tog);
    }
    selADC(mux);

  } // if isReady

  // Allows 10ms per conversion with 1 conversions for each of 4 devices.
  // So, joystick USB data gets updated every 10ms, but each device only gets read every 40ms (25x per sec) 
  // At 128SPS, there should be a new ADC datum available every 8ms.
  DigiJoystick.delay(10); // delay while keeping joystick active
  
} // loop
