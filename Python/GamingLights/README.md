# GamingLights #
Program to parse and send light configuration to an Arduino running the
GamingLights program trough a serial port.

This way simple light effects can be started from the commandline

## Requirements ##
- Python 3
- Arduino with GamingLights running
- pyserial
- bitstring

Although I see no reason why this program should not run on Windows or
Mac it is only tested on Ubuntu and Raspian.

## Installation ##
It's recommended to use a virtualenv for running GamingLights.
Within the virtualenv of outside if not using virtualenv issue the
following command to install the required libraries:

`pip install -r requirements.txt`

## Input file format ##
The input file has a simple format, in `demo.txt` an example is given.
Every empty line, or line starting with `#` is ignored.

Lines start with space/tab separated light configurations, where an
`*` indicates ON and `-` indicates OFF. The number of light configurations
depends on the configuration of the Arduino program, but cannot be greater then
16 positions.

After the light configuration the delay to the next item is given in ms. T
The delay MUST be greater than 0 and MUST be lower than 65536.

An example line `* - * - - - 250` indicates that light 1 and 3 should be
on for 250ms and all other lights should be off.

## Running the command ##
`gaming_lights.py [-h] [-v] filename port` where filename is a text file with
the required format. The port is the serial port where the Arduino is connected
to. In most cases this should be /dev/ttyACM0.

## The Arduino resets itself ##
This is due to the fact that the OS pulls DTR low signaling the Arduino to reset
itself on the next connection, on Debian/Raspian the following command can be
issued to prevent this issue:

`stty -F /dev/ttyACM[x] -hupcl` where `[x]` is the port number.

