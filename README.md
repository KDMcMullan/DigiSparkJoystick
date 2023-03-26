# DigiSparkJoystick
Adding a joystick to DigiSpark

The desire was to refurbish an old radio control (R/C) transmitter as a USB joystick. The DigiSpark seemsed like an obvious choice of device, posessing some digital IO, some analog inputs, a USB port and supporting libraries.

The DigiSpark (rev 3.0) has only 6 IO pins. Of these, PB2, PB3, PB4, and PB5 are analog inputs. My handheld controller has an XY stick, a throttle stick and an aditional potentiaometer. Looking good so far.

Ermm... No! Pins PB3 and PB4 are required for USB comms and PB 5 makes for a poor analog input as it resets the micro when pulled too low. (Yes, reset can be disabled, but that means that one needs a high voltage programmer on subsequent attempts to flash a new program. I wasn't playing that game.)
That leaves only PB2 as an analog input and a joystick with only one axis is not very useful.
So, an ADS1115 4-channel I2C ADC has been attached to the DigiSpark's PB0 (SDA) and PB2 (SCL) pins, to give four analog joystick inputs. This leaves the LED output (PB1) free as a watchdog to visually ensure the program is running, and, of course, PB3 and PB4 are free for USB data.

At this time, no buttons are provisioned, but my controller is for a helicopter, and posesses no buttons. At a later time, I may add some with an I2C DIO expander. There are many available: PCF8575, MAX23017 and MAX7321 are popular candidates.

Do what you will with this; it was an exercise in getting 4 good analog inputs from a DigiSpark using an I2C ADC. It might have been less painful to simply code up an Arduino, but there'd be no fun in that.

Couple of photos enclosed of the setup, including the handheld. There are 6 LEDs in the handheld unit. These are selected by a shift register. It might be fun to add the DIO expander, then use that to drive the LEDs. Yeah. I hear you: it would be less painful to drive the LEDs directly from the IO expander...

V01, Ken McMullan, 12-Mar-2023
