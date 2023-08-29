# Adapt-FFB-Joy: A joystick adapter for Microsoft SideWinder Force Feedback Pro Joysticks #

Adapt-FFB-Joy is an adapter that converts Microsoft SideWinder joysticks (the ones with the old DB15 connector) to USB-C. This adapter and firmware allows users to enjoy the advanced force feedback features of the SideWinder joysticks on a Windows machine without need for installing any custom device drivers.

## Sparkfun Pro Micro Support ##

The original version of the Adapt-FFB-Joy firmware was built to run on a Teensy 2.0 with an Atmel Atmega 32U4 microcontroller. As of this writing, the Teensy 2.0 has long been retired. However, SparkFun continues to make a breakout board (called the [Pro Micro](https://www.sparkfun.com/products/12640)) that is *mostly* complatible with the original firmware. 

This fork of the firmware has been modified to target the Sparkfun Pro Micro instead of the Teensy 2.0. Pins have been reassigned to better suit the Pro Micro. I have also included a breakout PCB that should be very inexpensive and easy to assemble.

## Building and flashing the firmware in 2022 ##

This project was originally designed to build on the latest (and very old) [WinAVR-20100110](https://sourceforge.net/projects/winavr/files/WinAVR/20100110/). I was unsuccessful in getting these libraries to work on Windows 10, so instead, I built the firmware using a Windows 7 VM. Your mileage may vary. 

If you're just looking to flash the .hex onto the Pro Micro, I recommend using AVRDUDESS (avrdude GUI for Windows). I have included a screenshot of my flash settings below. 

![](downloads/avrdudess.png?raw=true)

You can find the latest .hex file compiled for the Pro Micro [here](downloads/adaptffbjoy-pro-micro.hex).

## Differences between this version and the original ##

In order to make the Pro Micro work with this firmware, I had to reassign the GPIO used for "Button 1". Please refer to the [schematic](downloads/../pcb/MS%20Sidewinder%20Adapter_SCH.pdf) to build your own! 

I have also included Gerber PCB files in case you want to build a breakout board of your own. 

![](downloads/pcb.png?raw=true)

## Legacy documentation ##

For more information, check out the original [Adapt-ffb-joy Wiki](https://github.com/tloimu/adapt-ffb-joy/blob/wiki/README.md)
