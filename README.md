# GamingLights #
An Arduino and Python program to control lights/LEDs from a simple text file from the console.
Made to control LED buttons on my Arcade machine based on the current emulator/program.

- See [./Arduino/GamingLights/GamingLights.ino] for the Arduino program
- See [./Python/GamingLights/README.md] for information about the Python program

## Configuration file format ##
    # This is an demonstration for
    # the input format, not that every empty line or line that
    # starts with '#' is ignored and also the spaces at the start and end
    # are trimmed
    
    # 1 2 3 4 5 6 Delay in ms
      * - - - - - 250
      - * - - - - 250
      - - * - - - 250
      - - - * - - 250
      - - - - * - 250
      - - - - - * 250
      * * * * * * 100
      - - - - - - 100
      * * * * * * 100
      - - - - - - 100
      * * * * * * 100
      - - - - - - 100
      * * * * * * 100
      - - - - - - 100
