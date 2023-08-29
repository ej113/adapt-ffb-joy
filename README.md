# What is Adapt-FFB-Joy #

Adapt-FFB-Joy is an AVR microcontroller based device that looks like a joystick with advanced force feedback features in a Windows machine without need for installing any device drivers to PC.

This project contains the software for the AVR microcontroller as well as basic instructions for [building the hardware](https://github.com/tloimu/adapt-ffb-joy/blob/wiki/HowToBuild.md).

Currently, it allows connecting a Microsoft Sidewinder Force Feedback Pro (FFP) joystick (with a game port connector) to various MS Windows versions as a standard **USB joystick with force feedback** and **no need to install any device drivers**. The adapter also allows to solder a few additional trim pots to work e.g. as elevator trims, aileron trims and rudder pedals in your favorite simulator game.

For more information, see [Adapt-ffb-joy Wiki](https://github.com/tloimu/adapt-ffb-joy/blob/wiki/README.md)

The firmware software project is configured to compile for ATmega32U4 with WinAVR-20100110 or newer version.

# What is this fork? #
The objective of this fork is to make some improvements to the force feedback calls to the FFP to get as close as possible to a full implementation of the USB PID spec (as far as the FFP allows). This will include some bug fixes as well as support for FFP features not currently working. For more details see this [discussion thread](https://github.com/tloimu/adapt-ffb-joy/discussions/45)

**Branches:**

Master - isolated changes to improve the FFP force feedback code only

micro_pinout_and_mods - Pinout change for compatibility with Arduino Micro/Pro Micro boards; other mods included in juchong fork; FF code improvements 

micro_pinout_and_mods_LOG - As above with debug/logging enabled and some additional USB data retained for logging (I am using this for testing as I have an Arduino Micro but no Teensy 2.0)