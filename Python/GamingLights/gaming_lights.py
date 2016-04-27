#!/usr/bin/env python3
# Author: Elze Kool <info@kooldevelopment.nl>
# Licence: Attribution-NonCommercial-ShareAlike 4.0 International

"""
Program to parse and send light configuration to an Arduino running the GamingLights
program trough a serial port.

This way simple light effects can be started from the commandline

Copyright Elze Kool <info@kooldevelopment.nl>
"""

import pathlib
import re
from bitstring import BitArray
import serial
import argparse


def process_file(filename):
    """Process the file into an data array for sending to the Arduino

    Args:
        filename: The filename to read
    Returns:
        List with bytes to send
    """

    p = pathlib.Path(filename)

    if p.is_file() is False:
        print(filename, "does not exits")
        exit(2)

    # Create data buffer with start of input marker
    data = [0xFE]
    linecnt = 0

    with p.open() as f:
        for line in f:

            # Strip whitespace from line and replace tabs
            line = re.sub(' +', ' ', line.replace('\t', ' ').strip(' \r\n'))

            # Check if comment/empty line
            if len(line) == 0 or line[0] == '#':
                continue

            items = line.split(' ')

            # Validate if line could be split correctly
            if len(items) < 2:
                print("The line", line, "cannot be parsed, as it contains less than 2 elements")
                exit(3)

            # Check if we can parse the delay part of the line
            if items[-1].isnumeric() is False or int(items[-1]) <= 0:
                print("The line", line, "cannot be parsed, as it contains no valid delay")
                exit(3)

            # Check if the output configuration only contains valid characters
            if re.match('^[\*\-]+$', ''.join(items[0:-1])) is None:
                print("The line", line, "cannot be parsed, as it contains invalid output configuration")
                exit(3)

            # Convert data to format the Arduino expects
            output_conf = BitArray('0b' + ''.join(items[0:-1]).replace('*', '1').replace('-', '0')).uint
            delay_ms = int(items[-1])
            line_data = [
                (output_conf >> 8) & 0xff,
                output_conf & 0xff,
                (delay_ms >> 8) & 0xff,
                delay_ms & 0xff,
                linecnt
            ]

            # Add data to buffer
            data.extend(line_data)
            linecnt += 1

    # Add end of input mark
    data.extend([0xFF, 0xFF, 0xFF, 0xFF, 0xFF])

    return data


def main():
    """ Main function"""

    # Parse arguments
    parser = argparse.ArgumentParser(
        description='''Program to parse and send light configuration to an Arduino running the GamingLights

                       program trough a serial port.''',
        epilog='''Copyright 2016, Elze Kool <info@kooldevelopment.nl>

                  If you are having problems with the Arduino resetting itself, issue the following command:
                  stty -F /dev/ttyACM[x] -hupcl'''
    )
    parser.add_argument('filename', action='store', help='The file to process')
    parser.add_argument('port', action='store', help='The serial port to connect to')
    parser.add_argument('-v', '--version', action='version', version='GamingLights 1.0.0 by Elze Kool')

    args = parser.parse_args()

    # configure the serial connections (the parameters differs on the device you are connecting to)
    ser = serial.Serial(
        port=str(args.port),
        baudrate=115200,
        parity=serial.PARITY_NONE,
        stopbits=serial.STOPBITS_ONE,
        bytesize=serial.EIGHTBITS
    )

    # Process file
    data = process_file(args.filename)

    if ser.isOpen() is False:
        try:
            ser.open()
        except Exception as e:
            print("Error open serial port: " + str(e))
            exit(4)

        if ser.isOpen() is False:
            print("Error open serial port, port not open")
            exit(4)

    for i in data:
        ser.write(bytes([i]))

    ser.close()


if __name__ == "__main__":
    main()
